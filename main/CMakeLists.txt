set(COMPONENT_SRCS "poc.c"
		"uart.c"
		"http_server.c"
		"wifi.c"
		"ota.c"
		"mqtt.c")

set(COMPONENT_ADD_INCLUDEDIRS ".")

# Embed the server root certificate into the final binary
set(COMPONENT_EMBED_TXTFILES ${PROJECT_PATH}/server_certs/ota_ca_cert.pem ${PROJECT_PATH}/server_certs/iot_eclipse_org.pem)

register_component()
