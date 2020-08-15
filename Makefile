
MAKE = make

LIBKERNEL = asp_baseplatform/OBJ/K210_GCC/libkernel/libkernel.a
LIBWOLFSSL = wolfssl-4.4.0/Debug/libwolfssl.a
LIBZLIB = zlib-1.2.11/Debug/libzlib.a
LIBAZURE_IOT_SDK = azure_iot_sdk/Debug/libazure_iot_sdk.a
ASP_ELF = app_iothub_client/Debug/asp.elf

$(LIBKERNEL):
	$(MAKE) -j 1 -C asp_baseplatform/OBJ/K210_GCC/DEMO all

$(LIBWOLFSSL):
	$(MAKE) -j -C wolfssl-4.4.0 all

$(LIBZLIB):
	$(MAKE) -j -C zlib-1.2.11 all

$(LIBAZURE_IOT_SDK):
	$(MAKE) -j -C azure_iot_sdk all

$(ASP_ELF): $(LIBKERNEL) $(LIBWOLFSSL) $(LIBZLIB) $(LIBAZURE_IOT_SDK)
	$(MAKE) -j 1 -C app_iothub_client/Debug all

.PHONY: refresh
refresh: 
	rm -f $(LIBKERNEL)
	rm -f $(LIBWOLFSSL)
	rm -f $(LIBZLIB)
	rm -f $(LIBAZURE_IOT_SDK)
	rm -f $(ASP_ELF)

.PHONY: all
all: refresh $(ASP_ELF)

.PHONY: clean
clean:
	$(MAKE) -j -C asp_baseplatform/OBJ/K210_GCC/DEMO clean
	$(MAKE) -j -C wolfssl-4.4.0 clean
	$(MAKE) -j -C zlib-1.2.11 clean
	$(MAKE) -j -C azure_iot_sdk clean
	$(MAKE) -j -C app_iothub_client/Debug clean

.PHONY: realclean
realclean:
	$(MAKE) -j -C asp_baseplatform/OBJ/K210_GCC/DEMO realclean
	$(MAKE) -j -C wolfssl-4.4.0 clean
	$(MAKE) -j -C zlib-1.2.11 clean
	$(MAKE) -j -C azure_iot_sdk clean
	$(MAKE) -j -C app_iothub_client/Debug realclean
