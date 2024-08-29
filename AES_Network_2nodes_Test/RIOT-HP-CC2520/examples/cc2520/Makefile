# name of your application
APPLICATION = cc2520

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..


USEMODULE += xtimer
USEMODULE += luid
USEMODULE += ieee802154
USEMODULE += netdev_ieee802154
FEATURES_REQUIRED += periph_gpio
FEATURES_REQUIRED += periph_gpio_irq
FEATURES_REQUIRED += periph_spi

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
