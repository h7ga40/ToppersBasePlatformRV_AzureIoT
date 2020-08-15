// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef ESP_AT_SOCKET_H
#define ESP_AT_SOCKET_H

#ifdef __cplusplus
extern "C" {
#endif

	typedef void* ESP_AT_SOCKET_HANDLE;

	ESP_AT_SOCKET_HANDLE esp_at_socket_create(bool ssl);
	void esp_at_socket_set_blocking(ESP_AT_SOCKET_HANDLE espAtSocketHandle, bool blocking, unsigned int timeout);
	void esp_at_socket_destroy(ESP_AT_SOCKET_HANDLE espAtSocketHandle);
	int esp_at_socket_connect(ESP_AT_SOCKET_HANDLE espAtSocketHandle, const char* host, const int port);
	bool esp_at_socket_is_connected(ESP_AT_SOCKET_HANDLE espAtSocketHandle);
	void esp_at_socket_close(ESP_AT_SOCKET_HANDLE espAtSocketHandle);
	int esp_at_socket_send(ESP_AT_SOCKET_HANDLE espAtSocketHandle, const char* data, int length);
	int esp_at_socket_receive(ESP_AT_SOCKET_HANDLE espAtSocketHandle, char* data, int length);

#ifdef __cplusplus
}
#endif

#endif /* ESP_AT_SOCKET_H */
