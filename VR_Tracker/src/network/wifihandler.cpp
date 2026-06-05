/*
	SlimeVR Code is placed under the MIT license
	Copyright (c) 2021 Eiren Rain & SlimeVR contributors

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/
#include "network/wifihandler.h"

#include "GlobalVars.h"
#include "globals.h"
#if !ESP8266
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#endif

#if !ESP8266
// Log the real disconnect reason from ESP-IDF — much more informative than WL_DISCONNECTED
static void wifiDisconnectEventHandler(
	arduino_event_id_t event,
	arduino_event_info_t info
) {
	uint8_t reason = info.wifi_sta_disconnected.reason;
	// Reason codes from esp_wifi_types.h: 1=unspecified, 2=auth_expire, 4=auth_leave,
	// 15=4way_handshake_timeout (wrong password!), 200=beacon_timeout, 201=no_ap_found
	Serial.printf(
		"[DEBUG] [WiFiHandler] Disconnect event: reason=%d (%s)\n",
		reason,
		reason == 15 ? "WRONG_PASSWORD/4WAY_HANDSHAKE_TIMEOUT"
		: reason == 201 ? "NO_AP_FOUND"
		: reason == 200 ? "BEACON_TIMEOUT"
		: reason == 2   ? "AUTH_EXPIRE"
		: reason == 1   ? "UNSPECIFIED"
		: "see esp_wifi_types.h"
	);
}
#endif

namespace SlimeVR {

void WiFiNetwork::reportWifiProgress() {
	if (lastWifiReportTime + 1000 < millis()) {
		lastWifiReportTime = millis();
		Serial.print(".");
	}
}

void WiFiNetwork::setStaticIPIfDefined() {
#ifdef WIFI_USE_STATICIP
	const IPAddress ip(WIFI_STATIC_IP);
	const IPAddress gateway(WIFI_STATIC_GATEWAY);
	const IPAddress subnet(WIFI_STATIC_SUBNET);
	WiFi.config(ip, gateway, subnet);
#endif
}

bool WiFiNetwork::isConnected() const {
	return wifiState == WiFiReconnectionStatus::Success;
}

void WiFiNetwork::setWiFiCredentials(const char* SSID, const char* pass) {
	wifiProvisioning.stopProvisioning();
	tryConnecting(false, SSID, pass);
	retriedOnG = false;
	// Reset state, will get back into provisioning if can't connect
	hadWifi = false;
	wifiState = WiFiReconnectionStatus::ServerCredAttempt;
}

IPAddress WiFiNetwork::getAddress() { return WiFi.localIP(); }

void WiFiNetwork::setUp() {
	wifiHandlerLogger.info("=== WiFi setUp() start ===");
	WiFi.persistent(true);
	WiFi.mode(WIFI_STA);
	WiFi.hostname("SlimeVR FBT Tracker");
#if !ESP8266
	// Register disconnect event handler to log real reason code (e.g. wrong password = reason 15)
	WiFi.onEvent(wifiDisconnectEventHandler, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
#endif

	// Apply power saving BEFORE first WiFi.begin() so it takes effect immediately
#if ESP8266
#if POWERSAVING_MODE == POWER_SAVING_NONE
	WiFi.setSleepMode(WIFI_NONE_SLEEP);
#elif POWERSAVING_MODE == POWER_SAVING_MINIMUM
	WiFi.setSleepMode(WIFI_MODEM_SLEEP);
#elif POWERSAVING_MODE == POWER_SAVING_MODERATE
	WiFi.setSleepMode(WIFI_MODEM_SLEEP, 10);
#elif POWERSAVING_MODE == POWER_SAVING_MAXIMUM
	WiFi.setSleepMode(WIFI_LIGHT_SLEEP, 10);
#error "MAX POWER SAVING NOT WORKING YET, please disable!"
#endif
#else
#if POWERSAVING_MODE == POWER_SAVING_NONE
	esp_wifi_set_ps(WIFI_PS_NONE);
#elif POWERSAVING_MODE == POWER_SAVING_MINIMUM
	WiFi.setSleep(WIFI_PS_MIN_MODEM);
#elif POWERSAVING_MODE == POWER_SAVING_MODERATE \
	|| POWERSAVING_MODE == POWER_SAVING_MAXIMUM
	wifi_config_t conf;
	if (esp_wifi_get_config(WIFI_IF_STA, &conf) == ESP_OK) {
		conf.sta.listen_interval = 10;
		esp_wifi_set_config(WIFI_IF_STA, &conf);
		WiFi.setSleep(WIFI_PS_MAX_MODEM);
	} else {
		wifiHandlerLogger.error("Unable to get WiFi config, power saving not enabled!");
	}
#endif
#endif

	String ssid = getSSID();
	String pass = getPassword();
	wifiHandlerLogger.info(
		"Saved credentials: SSID='%s' passLen=%d",
		ssid.c_str(),
		pass.length()
	);

#if defined(WIFI_CREDS_SSID) && defined(WIFI_CREDS_PASSWD)
	wifiHandlerLogger.info(
		"Hardcoded credentials: SSID='%s' passLen=%d",
		WIFI_CREDS_SSID,
		strlen(WIFI_CREDS_PASSWD)
	);
#else
	wifiHandlerLogger.warn("No hardcoded WiFi credentials defined (WIFI_CREDS_SSID/PASSWD)!");
#endif

	wifiHandlerLogger.info("Calling trySavedCredentials()...");
	trySavedCredentials();
}

void WiFiNetwork::onConnected() {
	wifiState = WiFiReconnectionStatus::Success;
	wifiProvisioning.stopProvisioning();
	statusManager.setStatus(SlimeVR::Status::WIFI_CONNECTING, false);
	hadWifi = true;
	wifiHandlerLogger.info(
		"Connected successfully to SSID '%s', IP address %s",
		getSSID().c_str(),
		WiFi.localIP().toString().c_str()
	);
	// Reset it, in case we just connected with server creds
}

String WiFiNetwork::getSSID() {
#if ESP8266
	return WiFi.SSID();
#else
	// Necessary, because without a WiFi.begin(), ESP32 is not kind enough to load the
	// SSID on its own, for whatever reason
	wifi_config_t wifiConfig;
	esp_wifi_get_config((wifi_interface_t)ESP_IF_WIFI_STA, &wifiConfig);
	return {reinterpret_cast<char*>(wifiConfig.sta.ssid)};
#endif
}

String WiFiNetwork::getPassword() {
#if ESP8266
	return WiFi.psk();
#else
	// Same as above
	wifi_config_t wifiConfig;
	esp_wifi_get_config((wifi_interface_t)ESP_IF_WIFI_STA, &wifiConfig);
	return {reinterpret_cast<char*>(wifiConfig.sta.password)};
#endif
}

WiFiNetwork::WiFiReconnectionStatus WiFiNetwork::getWiFiState() { return wifiState; }

void WiFiNetwork::upkeep() {
	wifiProvisioning.upkeepProvisioning();

	if (WiFi.status() == WL_CONNECTED) {
		if (!isConnected()) {
			onConnected();
			return;
		}

		if (millis() - lastRssiSample >= 2000) {
			lastRssiSample = millis();
			uint8_t signalStrength = WiFi.RSSI();
			networkConnection.sendSignalStrength(signalStrength);
		}
		return;
	}

	if (isConnected()) {
		statusManager.setStatus(SlimeVR::Status::WIFI_CONNECTING, true);
		wifiHandlerLogger.warn("Connection to WiFi lost, reconnecting...");
		trySavedCredentials();
		return;
	}

	if (wifiState != WiFiReconnectionStatus::Failed) {
		reportWifiProgress();
	}

	if (millis() - wifiConnectionTimeout
			< static_cast<uint32_t>(WiFiTimeoutSeconds * 1000)
		&& WiFi.status() == WL_DISCONNECTED) {
		return;
	}

	switch (wifiState) {
		case WiFiReconnectionStatus::NotSetup:
			wifiHandlerLogger.warn("upkeep(): state=NotSetup, WiFi not started");
			return;
		case WiFiReconnectionStatus::SavedAttempt:
			wifiHandlerLogger.info("upkeep(): SavedAttempt timed out (status=%d), trying next...", (int)WiFi.status());
			if (!trySavedCredentials()) {
				tryHardcodedCredentials();
			}
			return;
		case WiFiReconnectionStatus::HardcodeAttempt:
			wifiHandlerLogger.info("upkeep(): HardcodeAttempt timed out (status=%d)", (int)WiFi.status());
			if (!tryHardcodedCredentials()) {
				wifiHandlerLogger.error("upkeep(): hardcoded creds also failed -> Failed");
				wifiState = WiFiReconnectionStatus::Failed;
			}
			return;
		case WiFiReconnectionStatus::ServerCredAttempt:
			wifiHandlerLogger.info("upkeep(): ServerCredAttempt timed out (status=%d)", (int)WiFi.status());
			if (!tryServerCredentials()) {
				wifiHandlerLogger.error("upkeep(): server creds failed -> Failed");
				wifiState = WiFiReconnectionStatus::Failed;
			}
			return;
		case WiFiReconnectionStatus::Failed:  // Couldn't connect with second set of
											  // credentials or server credentials
// Return to the default PHY Mode N.
#if ESP8266
			if constexpr (USE_ATTENUATION) {
				WiFi.setOutputPower(20.0 - ATTENUATION_N);
			}
			WiFi.setPhyMode(WIFI_PHY_MODE_11N);
#endif
			// Start smart config
			if (!hadWifi && !WiFi.smartConfigDone()
				&& millis() - wifiConnectionTimeout
					   >= static_cast<uint32_t>(WiFiTimeoutSeconds * 1000)) {
				if (WiFi.status() != WL_IDLE_STATUS) {
					wifiHandlerLogger.error(
						"Can't connect from any credentials, error: %d, reason: %s.",
						static_cast<int>(statusToFailure(WiFi.status())),
						statusToReasonString(WiFi.status())
					);
					wifiConnectionTimeout = millis();
				}
				wifiProvisioning.startProvisioning();
			}
			return;
	}
}

const char* WiFiNetwork::statusToReasonString(wl_status_t status) {
	switch (status) {
		case WL_DISCONNECTED:
			return "Timeout";
#ifdef ESP8266
		case WL_WRONG_PASSWORD:
			return "Wrong password";
		case WL_CONNECT_FAILED:
			return "Connection failed";
#elif ESP32
		case WL_CONNECT_FAILED:
			return "Wrong password";
#endif

		case WL_NO_SSID_AVAIL:
			return "SSID not found";
		default:
			return "Unknown";
	}
}

WiFiNetwork::WiFiFailureReason WiFiNetwork::statusToFailure(wl_status_t status) {
	switch (status) {
		case WL_DISCONNECTED:
			return WiFiFailureReason::Timeout;
#ifdef ESP8266
		case WL_WRONG_PASSWORD:
			return WiFiFailureReason::WrongPassword;
#elif ESP32
		case WL_CONNECT_FAILED:
			return WiFiFailureReason::WrongPassword;
#endif

		case WL_NO_SSID_AVAIL:
			return WiFiFailureReason::SSIDNotFound;
		default:
			return WiFiFailureReason::Unknown;
	}
}

void WiFiNetwork::showConnectionAttemptFailed(const char* type) const {
	wifiHandlerLogger.error(
		"Can't connect from %s credentials, error: %d, reason: %s.",
		type,
		static_cast<int>(statusToFailure(WiFi.status())),
		statusToReasonString(WiFi.status())
	);
}

bool WiFiNetwork::trySavedCredentials() {
	String ssid = getSSID();
	wifiHandlerLogger.info("trySavedCredentials(): SSID='%s' len=%d", ssid.c_str(), ssid.length());
	if (ssid.length() == 0) {
		wifiHandlerLogger.warn("Saved SSID is empty -> skip to HardcodeAttempt");
		wifiState = WiFiReconnectionStatus::HardcodeAttempt;
		return false;
	}

	if (wifiState == WiFiReconnectionStatus::SavedAttempt) {
		showConnectionAttemptFailed("saved");

		if (WiFi.status() != WL_DISCONNECTED) {
			return false;
		}

		if (retriedOnG) {
			return false;
		}

		retriedOnG = true;
		wifiHandlerLogger.debug("Trying saved credentials with PHY Mode G...");
		return tryConnecting(true);
	}

	retriedOnG = false;

	wifiState = WiFiReconnectionStatus::SavedAttempt;
	return tryConnecting();
}

bool WiFiNetwork::tryHardcodedCredentials() {
	wifiHandlerLogger.info("tryHardcodedCredentials()");
#if defined(WIFI_CREDS_SSID) && defined(WIFI_CREDS_PASSWD)
	if (wifiState == WiFiReconnectionStatus::HardcodeAttempt) {
		showConnectionAttemptFailed("hardcoded");

		if (WiFi.status() != WL_DISCONNECTED) {
			return false;
		}

		if (retriedOnG) {
			return false;
		}

		retriedOnG = true;
		wifiHandlerLogger.debug("Trying hardcoded credentials with PHY Mode G...");
		// Don't need to save hardcoded credentials
		WiFi.persistent(false);
		auto result = tryConnecting(true, WIFI_CREDS_SSID, WIFI_CREDS_PASSWD);
		WiFi.persistent(true);
		return result;
	}

	retriedOnG = false;

	wifiState = WiFiReconnectionStatus::HardcodeAttempt;
	// Don't need to save hardcoded credentials
	WiFi.persistent(false);
	auto result = tryConnecting(false, WIFI_CREDS_SSID, WIFI_CREDS_PASSWD);
	WiFi.persistent(true);
	return result;
#else
	wifiState = WiFiReconnectionStatus::HardcodeAttempt;
	return false;
#endif
}

bool WiFiNetwork::tryServerCredentials() {
	if (WiFi.status() != WL_DISCONNECTED) {
		return false;
	}

	if (retriedOnG) {
		return false;
	}

	retriedOnG = true;

	return tryConnecting(true);
}

bool WiFiNetwork::tryConnecting(bool phyModeG, const char* SSID, const char* pass) {
	wifiHandlerLogger.info(
		"tryConnecting(): SSID='%s' phyModeG=%d",
		SSID ? SSID : "(saved)",
		(int)phyModeG
	);

#if ESP8266
	if (phyModeG) {
		WiFi.setPhyMode(WIFI_PHY_MODE_11G);
		if constexpr (USE_ATTENUATION) {
			WiFi.setOutputPower(20.0 - ATTENUATION_G);
		}
	} else {
		WiFi.setPhyMode(WIFI_PHY_MODE_11N);
		if constexpr (USE_ATTENUATION) {
			WiFi.setOutputPower(20.0 - ATTENUATION_N);
		}
	}
#else
	if (phyModeG) {
		// ESP32 doesn't support manual PHY mode selection.
		// Use this retry slot to attempt reconnection with a clean disconnect first.
		wifiHandlerLogger.debug("phyModeG not supported on ESP32 — retrying with clean disconnect");
		WiFi.disconnect(true);
		delay(200);
	}
#endif

	setStaticIPIfDefined();
	if (SSID == nullptr) {
		wifiHandlerLogger.info("WiFi.begin() with saved credentials");
		WiFi.begin();
	} else {
		wifiHandlerLogger.info("WiFi.begin(SSID='%s')", SSID);
		WiFi.begin(SSID, pass);
	}
	wifiConnectionTimeout = millis();
	return true;
}

}  // namespace SlimeVR
