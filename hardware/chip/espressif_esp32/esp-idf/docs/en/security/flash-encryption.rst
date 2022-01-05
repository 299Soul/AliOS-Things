Flash Encryption
================

:link_to_translation:`zh_CN:[中文]`

This is a quick start guide to {IDF_TARGET_NAME}'s flash encryption feature. Using an application code example, it demonstrates how to test and verify flash encryption operations during development and production.


Introduction
------------

Flash encryption is intended for encrypting the contents of the {IDF_TARGET_NAME}'s off-chip flash memory. Once this feature is enabled, firmware is flashed as plaintext, and then the data is encrypted in place on the first boot. As a result, physical readout of flash will not be sufficient to recover most flash contents.

With flash encryption enabled, the following types of data are encrypted by default:

- Firmware bootloader
- Partition Table
- All "app" type partitions

Other types of data can be encrypted conditionally:

- Any partition marked with the ``encrypted`` flag in the partition table. For details, see :ref:`encrypted-partition-flag`.
- Secure Boot bootloader digest if Secure Boot is enabled (see below).

.. only:: esp32

    :doc:`Secure Boot <secure-boot-v2>` is a separate feature which can be used together with flash encryption to create an even more secure environment.

.. important::
   For production use, flash encryption should be enabled in the "Release" mode only.


.. important::

    Enabling flash encryption limits the options for further updates of {IDF_TARGET_NAME}. Before using this feature, read the document and make sure to understand the implications.


.. _flash-encryption-efuse:

Relevant eFuses
---------------

The flash encryption operation is controlled by various eFuses available on {IDF_TARGET_NAME}. The list of eFuses and their descriptions is given in the table below.

.. Comment: As text in cells of list-table header rows does not wrap, it is necessary to make 0 header rows and apply bold typeface to the first row. Otherwise, the table goes beyond the html page limits on the right.

.. only:: esp32

    .. list-table:: eFuses Used in Flash Encryption
       :widths: 25 40 10 15 10
       :header-rows: 0

       * - **eFuse**
         - **Description**
         - **Bit Depth**
         - **Locking for Reading/Writing Available**
         - **Default Value**
       * - ``CODING_SCHEME``
         - Controls actual number of BLOCK1 bits used to derive final 256-bit AES key. Possible values: ``0`` for 256 bits, ``1`` for 192 bits, ``2`` for 128 bits. Final AES key is derived based on the ``FLASH_CRYPT_CONFIG`` value.
         - 2
         - Yes
         - 0
       * - ``BLOCK1``
         - AES key storage.
         - 256
         - Yes
         - x
       * - ``FLASH_CRYPT_CONFIG``
         - Controls the AES encryption process.
         - 4
         - Yes
         - 0xF
       * - ``download_dis_encrypt``
         - If set, disables flash encryption operation while running in Firmware Download mode.
         - 1
         - Yes
         - 0
       * - ``download_dis_decrypt``
         - If set, disables flash decryption while running in UART Firmware Download mode.
         - 1
         - Yes
         - 0
       * - ``FLASH_CRYPT_CNT``
         - Enables/disables encryption at boot time. If even number of bits set (0, 2, 4, 6) - encrypt flash at boot time. If odd number of bits set (1, 3, 5, 7) - do not encrypt flash at boot time.
         - 7
         - Yes
         - 0

    Read and write access to eFuse bits is controlled by appropriate fields in the registers ``efuse_wr_disable`` and ``efuse_rd_disable``. For more information on {IDF_TARGET_NAME} eFuses, see :doc:`eFuse manager <../api-reference/system/efuse>`.


.. only:: esp32s2

    .. list-table:: eFuses Used in Flash Encryption
       :widths: 25 40 10 15 10
       :header-rows: 0

       * - **eFuse**
         - **Description**
         - **Bit Depth**
         - **Locking for Reading/Writing Available**
         - **Default Value**
       * - ``KEYN``
         - AES key storage. N is between 0 and 5.
         - 256
         - Yes
         - x
       * - ``EFUSE_KEY_PURPOSE_N``
         - Controls the purpose of eFuse block ``KEYN``, where N is between 0 and 5. Possible values: ``2`` for ``XTS_AES_256_KEY_1`` , ``3`` for ``XTS_AES_256_KEY_2``, and ``4`` for ``XTS_AES_128_KEY``. Final AES key is derived based on the value of one or two of these purpose eFuses. For a detailed description of the possible combinations see `{IDF_TARGET_NAME} Technical Reference Manual <{IDF_TARGET_TRM_EN_URL}>`_, chapter Flash Encryption.
         - 4
         - Yes
         - 0
       * - ``EFUSE_DIS_DOWNLOAD_MANUAL_ENCRYPT``
         - If set, disables flash encryption when in download bootmodes.
         - 1
         - Yes
         - 0
       * - ``EFUSE_SPI_BOOT_CRYPT_CNT``
         - Enables encryption and decryption, when an SPI boot mode is set. Feature is enabled if 1 or 3 bits are set in the eFuse, disabled otherwise.
         - 3
         - Yes
         - 0

    Read and write access to eFuse bits is controlled by appropriate fields in the registers ``EFUSE_WR_DIS`` and ``EFUSE_RD_DIS``. For more information on {IDF_TARGET_NAME} eFuses, see :doc:`eFuse manager <../api-reference/system/efuse>`.


Flash Encryption Process
------------------------

{IDF_TARGET_CRYPT_CNT:default="EFUSE_SPI_BOOT_CRYPT_CNT",esp32="FLASH_CRYPT_CNT",esp32s2="EFUSE_SPI_BOOT_CRYPT_CNT"}

Assuming that the eFuse values are in their default states and the firmware bootloader is compiled to support flash encryption, the flash encryption process executes as shown below:

.. only:: esp32

  1. On the first power-on reset, all data in flash is un-encrypted (plaintext). The ROM bootloader loads the firmware bootloader.

  2. Firmware bootloader reads the ``FLASH_CRYPT_CNT`` eFuse value (``0b00000000``). Since the value is ``0`` (even number of bits set), it configures and enables the flash encryption block. It also sets the ``FLASH_CRYPT_CONFIG`` eFuse to 0xF. For more information on the flash encryption block, see `{IDF_TARGET_NAME} Technical Reference Manual <{IDF_TARGET_TRM_EN_URL}>`_.

  3. Flash encryption block generates an AES-256 bit key and writes it into the BLOCK1 eFuse. This operation is done entirely by hardware, and the key cannot be accessed via software.

  4. Flash encryption block encrypts the flash contents - partitions encrypted by default and the ones marked as ``encrypted``. Encrypting in-place can take time, up to a minute for large partitions.

  5. Firmware bootloader sets the first available bit in ``FLASH_CRYPT_CNT`` (0b00000001) to mark the flash contents as encrypted. Odd number of bits is set.

  6. For :ref:`flash-enc-development-mode`, the firmware bootloader sets only the eFuse bits ``download_dis_decrypt`` and ``download_dis_cache`` to allow the UART bootloader to re-flash encrypted binaries. Also, the ``FLASH_CRYPT_CNT`` eFuse bits are NOT write-protected.

  7. For :ref:`flash-enc-release-mode`, the firmware bootloader sets the eFuse bits ``download_dis_encrypt``, ``download_dis_decrypt``, and ``download_dis_cache`` to 1 to prevent the UART bootloader from decrypting the flash contents. It also write-protects the ``FLASH_CRYPT_CNT`` eFuse bits. To modify this behavior, see :ref:`uart-bootloader-encryption`.

  8. The device is then rebooted to start executing the encrypted image. The firmware bootloader calls the flash decryption block to decrypt the flash contents and then loads the decrypted contents into IRAM.

.. only:: esp32s2

  1. On the first power-on reset, all data in flash is un-encrypted (plaintext). The ROM bootloader loads the firmware bootloader.

  2. Firmware bootloader reads the ``EFUSE_SPI_BOOT_CRYPT_CNT`` eFuse value (``0b00000000``). Since the value is ``0`` (even number of bits set), it configures and enables the flash encryption block. For more information on the flash encryption block, see `{IDF_TARGET_NAME} Technical Reference Manual <{IDF_TARGET_TRM_EN_URL}>`_.

  3. Flash encryption block generates an 256 bit or 512 bit key, depending on the value of :ref:`Size of generated AES-XTS key <CONFIG_SECURE_FLASH_ENCRYPTION_KEYSIZE>`, and writes it into respectively one or two `KEYN` eFuses. The software also updates the ``EFUSE_KEY_PURPOSE_N`` for the blocks where the keys where stored. This operation is done entirely by hardware, and the key cannot be accessed via software.

  4. Flash encryption block encrypts the flash contents - partitions encrypted by default and the ones marked as ``encrypted``. Encrypting in-place can take time, up to a minute for large partitions.

  5. Firmware bootloader sets the first available bit in ``EFUSE_SPI_BOOT_CRYPT_CNT`` (0b00000001) to mark the flash contents as encrypted. Odd number of bits is set.

  6. For :ref:`flash-enc-development-mode`, the firmware bootloader allows the UART bootloader to re-flash encrypted binaries. Also, the ``EFUSE_SPI_BOOT_CRYPT_CNT`` eFuse bits are NOT write-protected.

  7. For :ref:`flash-enc-release-mode`, the firmware bootloader sets the eFuse bits ``EFUSE_DIS_DOWNLOAD_MANUAL_ENCRYPT``, ``EFUSE_DIS_BOOT_REMAP``, ``EFUSE_DIS_DOWNLOAD_ICACHE`` and ``EFUSE_DIS_DOWNLOAD_DCACHE``. It also write-protects the ``EFUSE_SPI_BOOT_CRYPT_CNT`` eFuse bits. To modify this behavior, see :ref:`uart-bootloader-encryption`.

  8. The device is then rebooted to start executing the encrypted image. The firmware bootloader calls the flash decryption block to decrypt the flash contents and then loads the decrypted contents into IRAM.


During the development stage, there is a frequent need to program different plaintext flash images and test the flash encryption process. This requires that Firmware Download mode is able to load new plaintext images as many times as it might be needed. However, during manufacturing or production stages, Firmware Download mode should not be allowed to access flash contents for security reasons.

Hence, two different flash encryption configurations were created: for development and for production. For details on these configurations, see Section `Flash Encryption Configuration`_.

.. _{IDF_TARGET_NAME} Technical Reference Manual: {IDF_TARGET_TRM_EN_URL}


Flash Encryption Configuration
------------------------------

The following flash encryption modes are available:

- :ref:`flash-enc-development-mode` - recommended for use ONLY DURING DEVELOPMENT, as it does not prevent modification and possible readout of encrypted flash contents.
- :ref:`flash-enc-release-mode` - recommended for manufacturing and production to prevent physical readout of encrypted flash contents.


This section provides information on the mentioned flash encryption modes and step by step instructions on how to use them.


.. _flash-enc-development-mode:

Development Mode
^^^^^^^^^^^^^^^^

During development, you can encrypt flash using either an {IDF_TARGET_NAME} generated key or external host-generated key.


Using {IDF_TARGET_NAME} Generated Key
"""""""""""""""""""""""""""""""""""""

Development mode allows you to download multiple plaintext images using Firmware Download mode.

To test flash encryption process, take the following steps:

1. Ensure that you have an {IDF_TARGET_NAME} device with default flash encryption eFuse settings as shown in :ref:`flash-encryption-efuse`.

  See how to check :ref:`flash-encryption-status`.

2. In :ref:`project-configuration-menu`, do the following:

  .. list::

    - :ref:`Enable flash encryption on boot <CONFIG_SECURE_FLASH_ENC_ENABLED>`
    - :ref:`Select encryption mode <CONFIG_SECURE_FLASH_ENCRYPTION_MODE>` (**Development mode** by default)
    :esp32s2: - Set :ref:`Size of generated AES-XTS key <CONFIG_SECURE_FLASH_ENCRYPTION_KEYSIZE>`
    - :ref:`Select the appropriate bootloader log verbosity <CONFIG_BOOTLOADER_LOG_LEVEL>`
    - Save the configuration and exit.

  .. only:: esp32

      Enabling flash encryption will increase the size of bootloader, which might require updating partition table offset. See :ref:`secure-boot-bootloader-size`

3. Run the command given below to build and flash the complete image.

  .. code-block:: bash

      idf.py flash monitor

  The image will include the firmware bootloader, partition table, application, and other partitions marked by the user as ``encrypted``. These binaries will be written to flash memory unencrypted. Once the flashing is complete, your device will reset. On the next boot, the firmware bootloader encrypts the flash application partition and then resets. After that, the sample application is decrypted at runtime and executed.

A sample output of the first {IDF_TARGET_NAME} boot after enabling flash encryption is given below:

.. code-block:: bash

    --- idf_monitor on /dev/cu.SLAB_USBtoUART 115200 ---
    --- Quit: Ctrl+] | Menu: Ctrl+T | Help: Ctrl+T followed by Ctrl+H ---
    ets Jun  8 2016 00:22:57

    rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
    configsip: 0, SPIWP:0xee
    clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
    mode:DIO, clock div:2
    load:0x3fff0018,len:4
    load:0x3fff001c,len:8452
    load:0x40078000,len:13608
    load:0x40080400,len:6664
    entry 0x40080764
    I (28) boot: ESP-IDF v4.0-dev-850-gc4447462d-dirty 2nd stage bootloader
    I (29) boot: compile time 15:37:14
    I (30) boot: Enabling RNG early entropy source...
    I (35) boot: SPI Speed      : 40MHz
    I (39) boot: SPI Mode       : DIO
    I (43) boot: SPI Flash Size : 4MB
    I (47) boot: Partition Table:
    I (51) boot: ## Label            Usage          Type ST Offset   Length
    I (58) boot:  0 nvs              WiFi data        01 02 0000a000 00006000
    I (66) boot:  1 phy_init         RF data          01 01 00010000 00001000
    I (73) boot:  2 factory          factory app      00 00 00020000 00100000
    I (81) boot: End of partition table
    I (85) esp_image: segment 0: paddr=0x00020020 vaddr=0x3f400020 size=0x0808c ( 32908) map
    I (105) esp_image: segment 1: paddr=0x000280b4 vaddr=0x3ffb0000 size=0x01ea4 (  7844) load
    I (109) esp_image: segment 2: paddr=0x00029f60 vaddr=0x40080000 size=0x00400 (  1024) load
    0x40080000: _WindowOverflow4 at esp-idf/esp-idf/components/freertos/xtensa_vectors.S:1778

    I (114) esp_image: segment 3: paddr=0x0002a368 vaddr=0x40080400 size=0x05ca8 ( 23720) load
    I (132) esp_image: segment 4: paddr=0x00030018 vaddr=0x400d0018 size=0x126a8 ( 75432) map
    0x400d0018: _flash_cache_start at ??:?

    I (159) esp_image: segment 5: paddr=0x000426c8 vaddr=0x400860a8 size=0x01f4c (  8012) load
    0x400860a8: prvAddNewTaskToReadyList at esp-idf/esp-idf/components/freertos/tasks.c:4561

    I (168) boot: Loaded app from partition at offset 0x20000
    I (168) boot: Checking flash encryption...
    I (168) flash_encrypt: Generating new flash encryption key...
    I (187) flash_encrypt: Read & write protecting new key...
    I (187) flash_encrypt: Setting CRYPT_CONFIG efuse to 0xF
    W (188) flash_encrypt: Not disabling UART bootloader encryption
    I (195) flash_encrypt: Disable UART bootloader decryption...
    I (201) flash_encrypt: Disable UART bootloader MMU cache...
    I (208) flash_encrypt: Disable JTAG...
    I (212) flash_encrypt: Disable ROM BASIC interpreter fallback...
    I (219) esp_image: segment 0: paddr=0x00001020 vaddr=0x3fff0018 size=0x00004 (     4)
    I (227) esp_image: segment 1: paddr=0x0000102c vaddr=0x3fff001c size=0x02104 (  8452)
    I (239) esp_image: segment 2: paddr=0x00003138 vaddr=0x40078000 size=0x03528 ( 13608)
    I (249) esp_image: segment 3: paddr=0x00006668 vaddr=0x40080400 size=0x01a08 (  6664)
    I (657) esp_image: segment 0: paddr=0x00020020 vaddr=0x3f400020 size=0x0808c ( 32908) map
    I (669) esp_image: segment 1: paddr=0x000280b4 vaddr=0x3ffb0000 size=0x01ea4 (  7844)
    I (672) esp_image: segment 2: paddr=0x00029f60 vaddr=0x40080000 size=0x00400 (  1024)
    0x40080000: _WindowOverflow4 at esp-idf/esp-idf/components/freertos/xtensa_vectors.S:1778

    I (676) esp_image: segment 3: paddr=0x0002a368 vaddr=0x40080400 size=0x05ca8 ( 23720)
    I (692) esp_image: segment 4: paddr=0x00030018 vaddr=0x400d0018 size=0x126a8 ( 75432) map
    0x400d0018: _flash_cache_start at ??:?

    I (719) esp_image: segment 5: paddr=0x000426c8 vaddr=0x400860a8 size=0x01f4c (  8012)
    0x400860a8: prvAddNewTaskToReadyList at esp-idf/esp-idf/components/freertos/tasks.c:4561

    I (722) flash_encrypt: Encrypting partition 2 at offset 0x20000...
    I (13229) flash_encrypt: Flash encryption completed
    I (13229) boot: Resetting with flash encryption enabled...

A sample output of subsequent {IDF_TARGET_NAME} boots just mentions that flash encryption is already enabled:

.. code-block:: bash

    rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
    configsip: 0, SPIWP:0xee
    clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
    mode:DIO, clock div:2
    load:0x3fff0018,len:4
    load:0x3fff001c,len:8452
    load:0x40078000,len:13652
    ho 0 tail 12 room 4
    load:0x40080400,len:6664
    entry 0x40080764
    I (30) boot: ESP-IDF v4.0-dev-850-gc4447462d-dirty 2nd stage bootloader
    I (30) boot: compile time 16:32:53
    I (31) boot: Enabling RNG early entropy source...
    I (37) boot: SPI Speed      : 40MHz
    I (41) boot: SPI Mode       : DIO
    I (45) boot: SPI Flash Size : 4MB
    I (49) boot: Partition Table:
    I (52) boot: ## Label            Usage          Type ST Offset   Length
    I (60) boot:  0 nvs              WiFi data        01 02 0000a000 00006000
    I (67) boot:  1 phy_init         RF data          01 01 00010000 00001000
    I (75) boot:  2 factory          factory app      00 00 00020000 00100000
    I (82) boot: End of partition table
  I (86) esp_image: segment 0: paddr=0x00020020 vaddr=0x3f400020 size=0x0808c ( 32908) map
    I (107) esp_image: segment 1: paddr=0x000280b4 vaddr=0x3ffb0000 size=0x01ea4 (  7844) load
    I (111) esp_image: segment 2: paddr=0x00029f60 vaddr=0x40080000 size=0x00400 (  1024) load
    0x40080000: _WindowOverflow4 at esp-idf/esp-idf/components/freertos/xtensa_vectors.S:1778

    I (116) esp_image: segment 3: paddr=0x0002a368 vaddr=0x40080400 size=0x05ca8 ( 23720) load
    I (134) esp_image: segment 4: paddr=0x00030018 vaddr=0x400d0018 size=0x126a8 ( 75432) map
    0x400d0018: _flash_cache_start at ??:?

    I (162) esp_image: segment 5: paddr=0x000426c8 vaddr=0x400860a8 size=0x01f4c (  8012) load
    0x400860a8: prvAddNewTaskToReadyList at esp-idf/esp-idf/components/freertos/tasks.c:4561

    I (171) boot: Loaded app from partition at offset 0x20000
    I (171) boot: Checking flash encryption...
    I (171) flash_encrypt: flash encryption is enabled (3 plaintext flashes left)
    I (178) boot: Disabling RNG early entropy source...
    I (184) cpu_start: Pro cpu up.
    I (188) cpu_start: Application information:
    I (193) cpu_start: Project name:     flash-encryption
    I (198) cpu_start: App version:      v4.0-dev-850-gc4447462d-dirty
    I (205) cpu_start: Compile time:     Jun 17 2019 16:32:52
    I (211) cpu_start: ELF file SHA256:  8770c886bdf561a7...
    I (217) cpu_start: ESP-IDF:          v4.0-dev-850-gc4447462d-dirty
    I (224) cpu_start: Starting app cpu, entry point is 0x40080e4c
    0x40080e4c: call_start_cpu1 at esp-idf/esp-idf/components/{IDF_TARGET_PATH_NAME}/cpu_start.c:265

    I (0) cpu_start: App cpu up.
    I (235) heap_init: Initializing. RAM available for dynamic allocation:
    I (241) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
    I (247) heap_init: At 3FFB2EC8 len 0002D138 (180 KiB): DRAM
    I (254) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
    I (260) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
    I (266) heap_init: At 40087FF4 len 0001800C (96 KiB): IRAM
    I (273) cpu_start: Pro cpu start user code
    I (291) cpu_start: Starting scheduler on PRO CPU.
    I (0) cpu_start: Starting scheduler on APP CPU.

    Sample program to check Flash Encryption
    This is ESP32 chip with 2 CPU cores, WiFi/BT/BLE, silicon revision 1, 4MB external flash
    Flash encryption feature is enabled
    Flash encryption mode is DEVELOPMENT
    Flash in encrypted mode with flash_crypt_cnt = 1
    Halting...

At this stage, if you need to update and re-flash binaries, see :ref:`encrypt-partitions`.


.. _pregenerated-flash-encryption-key:

Using Host Generated Key
""""""""""""""""""""""""

It is possible to pre-generate a flash encryption key on the host computer and burn it into the eFuse. This allows you to pre-encrypt data on the host and flash already encrypted data without needing a plaintext flash update. This feature can be used in both :ref:`flash-enc-development-mode` and :ref:`flash-enc-release-mode`. Without a pre-generated key, data is flashed in plaintext and then {IDF_TARGET_NAME} encrypts the data in-place.

.. note::

    This option is not recommended for production, unless a separate key is generated for each individual device.

To use a host generated key, take the following steps:

1. Ensure that you have an {IDF_TARGET_NAME} device with default flash encryption eFuse settings as shown in :ref:`flash-encryption-efuse`.

  See how to check :ref:`flash-encryption-status`.

2. Generate a random key by running:

  .. code-block:: bash

      espsecure.py generate_flash_encryption_key my_flash_encryption_key.bin

3. **Before the first encrypted boot**, burn the key into your device's BLOCK1 eFuse using the command below. This action can be done **only once**.

  .. code-block:: bash

      espefuse.py --port PORT burn_key flash_encryption my_flash_encryption_key.bin

  If the key is not burned and the device is started after enabling flash encryption, the {IDF_TARGET_NAME} will generate a random key that software cannot access or modify.

4. In :ref:`project-configuration-menu`, do the following:

    - :ref:`Enable flash encryption on boot <CONFIG_SECURE_FLASH_ENC_ENABLED>`
    - :ref:`Select encryption mode <CONFIG_SECURE_FLASH_ENCRYPTION_MODE>` (**Development mode** by default)
    - :ref:`Select the appropriate bootloader log verbosity <CONFIG_BOOTLOADER_LOG_LEVEL>`
    - Save the configuration and exit.

  .. only:: esp32

      Enabling flash encryption will increase the size of bootloader, which might require updating partition table offset. See :ref:`secure-boot-bootloader-size`

5. Run the command given below to build and flash the complete.

  .. code-block:: bash

      idf.py flash monitor

  The image will include the firmware bootloader, partition table, application, and other partitions marked by the user as ``encrypted``. These binaries will be written to flash memory unencrypted. Once the flashing is complete, your device will reset. On the next boot, the firmware bootloader encrypts the flash application partition and then resets. After that, the sample application is decrypted at runtime and executed.

At this stage, if you need to update and re-flash binaries, see :ref:`encrypt-partitions`.


.. _encrypt-partitions:

Re-flashing Updated Partitions
""""""""""""""""""""""""""""""

If you update your application code (done in plaintext) and want to re-flash it, you will need to encrypt it before flashing. To encrypt the application and flash it in one step, run:

.. code-block:: bash

    idf.py encrypted-app-flash monitor

If all partitions needs to be updated in encrypted format, run:

.. code-block:: bash

    idf.py encrypted-flash monitor


.. _flash-enc-release-mode:

Release Mode
^^^^^^^^^^^^

In Release mode, UART bootloader cannot perform flash encryption operations. New plaintext images can ONLY be downloaded using the over-the-air (OTA) scheme which will encrypt the plaintext image before writing to flash.

To use this mode, take the following steps:

1. Ensure that you have an {IDF_TARGET_NAME} device with default flash encryption eFuse settings as shown in :ref:`flash-encryption-efuse`.

  See how to check :ref:`flash-encryption-status`.

2. In :ref:`project-configuration-menu`, do the following:

  .. list::

    - :ref:`Enable flash encryption on boot <CONFIG_SECURE_FLASH_ENC_ENABLED>`
    :esp32: - :ref:`Select Release mode <CONFIG_SECURE_FLASH_ENCRYPTION_MODE>` (Note that once Release mode is selected, the ``download_dis_encrypt`` and ``download_dis_decrypt`` eFuse bits will be burned to disable UART bootloader access to flash contents)
    :esp32s2: - :ref:`Select Release mode <CONFIG_SECURE_FLASH_ENCRYPTION_MODE>` (Note that once Release mode is selected, the ``EFUSE_DIS_DOWNLOAD_MANUAL_ENCRYPT`` eFuse bit will be burned to disable UART bootloader access to flash contents)
    :esp32s2: - Set :ref:`Size of generated AES-XTS key <CONFIG_SECURE_FLASH_ENCRYPTION_KEYSIZE>`
    - :ref:`Select the appropriate bootloader log verbosity <CONFIG_BOOTLOADER_LOG_LEVEL>`
    - Save the configuration and exit.

  .. only:: esp32

      Enabling flash encryption will increase the size of bootloader, which might require updating partition table offset. See :ref:`secure-boot-bootloader-size`

3. Run the command given below to build and flash the complete image.

  .. code-block:: bash

      idf.py flash monitor

  The image will include the firmware bootloader, partition table, application, and other partitions marked by the user as ``encrypted``. These binaries will be written to flash memory unencrypted. Once the flashing is complete, your device will reset. On the next boot, the firmware bootloader encrypts the flash application partition and then resets. After that, the sample application is decrypted at runtime and executed.

Once the flash encryption is enabled in Release mode, the bootloader will write-protect the ``{IDF_TARGET_CRYPT_CNT}`` eFuse.

For subsequent plaintext field updates, use :ref:`OTA scheme <updating-encrypted-flash-ota>`.


. _flash-encrypt-best-practices:

Best Practices
^^^^^^^^^^^^^^

When using Flash Encryption in production:

.. list::

   - Do not reuse the same flash encryption key between multiple devices. This means that an attacker who copies encrypted data from one device cannot transfer it to a second device.
   :esp32: - When using ESP32 V3, if the UART ROM Download Mode is not needed for a production device then it should be disabled to provide an extra level of protection. Do this by calling :cpp:func:`esp_efuse_disable_rom_download_mode` during application startup. Alternatively, configure the project :ref:`CONFIG_ESP32_REV_MIN` level to 3 (targeting ESP32 V3 only) and enable :ref:`CONFIG_SECURE_DISABLE_ROM_DL_MODE`. The ability to disable ROM Download Mode is not available on earlier ESP32 versions.
   :esp32s2: - The UART ROM Download Mode should be disabled entirely if it is not needed, or permanently set to "Secure Download Mode" otherwise. Secure Download Mode permanently limits the available commands to basic flash read and write only. The default behaviour is to set Secure Download Mode on first boot in Release mode. To disable Download Mode entirely, enable configuration option :ref:`CONFIG_SECURE_DISABLE_ROM_DL_MODE` or call :cpp:func:`esp_efuse_disable_rom_download_mode` at runtime.
   :esp32: -  Enable :doc:`Secure Boot <secure-boot-v2>` as an extra layer of protection, and to prevent an attacker from selectively corrupting any part of the flash before boot.
   :esp32s2: -  Enable Secure Boot as an extra layer of protection, and to prevent an attacker from selectively corrupting any part of the flash before boot.

Possible Failures
-----------------

Once flash encryption is enabled, the ``{IDF_TARGET_CRYPT_CNT}`` eFuse value will have an odd number of bits set. It means that all the partitions marked with the encryption flag are expected to contain encrypted ciphertext. Below are the three typical failure cases if the {IDF_TARGET_NAME} is erroneously loaded with plaintext data:

1. If the bootloader partition is re-flashed with a **plaintext firmware bootloader image**, the ROM bootloader will fail to load the firmware bootloader resulting in the following failure:

  .. code-block:: bash

      rst:0x3 (SW_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
      flash read err, 1000
      ets_main.c 371
      ets Jun  8 2016 00:22:57

      rst:0x7 (TG0WDT_SYS_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
      flash read err, 1000
      ets_main.c 371
      ets Jun  8 2016 00:22:57

      rst:0x7 (TG0WDT_SYS_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
      flash read err, 1000
      ets_main.c 371
      ets Jun  8 2016 00:22:57

      rst:0x7 (TG0WDT_SYS_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
      flash read err, 1000
      ets_main.c 371
      ets Jun  8 2016 00:22:57

      rst:0x7 (TG0WDT_SYS_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
      flash read err, 1000
      ets_main.c 371
      ets Jun  8 2016 00:22:57

.. note::

    This error also appears if the flash contents are erased or corrupted.

2. If the firmware bootloader is encrypted, but the partition table is re-flashed with a **plaintext partition table image**, the bootloader will fail to read the partition table resulting in the following failure:

  .. code-block:: bash

      rst:0x3 (SW_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
      configsip: 0, SPIWP:0xee
      clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
      mode:DIO, clock div:2
      load:0x3fff0018,len:4
      load:0x3fff001c,len:10464
      ho 0 tail 12 room 4
      load:0x40078000,len:19168
      load:0x40080400,len:6664
      entry 0x40080764
      I (60) boot: ESP-IDF v4.0-dev-763-g2c55fae6c-dirty 2nd stage bootloader
      I (60) boot: compile time 19:15:54
      I (62) boot: Enabling RNG early entropy source...
      I (67) boot: SPI Speed      : 40MHz
      I (72) boot: SPI Mode       : DIO
      I (76) boot: SPI Flash Size : 4MB
      E (80) flash_parts: partition 0 invalid magic number 0x94f6
      E (86) boot: Failed to verify partition table
      E (91) boot: load partition table error!

3. If the bootloader and partition table are encrypted, but the application is re-flashed with a **plaintext application image**, the bootloader will fail to load the application resulting in the following failure:

  .. code-block:: bash

      rst:0x3 (SW_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
      configsip: 0, SPIWP:0xee
      clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
      mode:DIO, clock div:2
      load:0x3fff0018,len:4
      load:0x3fff001c,len:8452
      load:0x40078000,len:13616
      load:0x40080400,len:6664
      entry 0x40080764
      I (56) boot: ESP-IDF v4.0-dev-850-gc4447462d-dirty 2nd stage bootloader
      I (56) boot: compile time 15:37:14
      I (58) boot: Enabling RNG early entropy source...
      I (64) boot: SPI Speed      : 40MHz
      I (68) boot: SPI Mode       : DIO
      I (72) boot: SPI Flash Size : 4MB
      I (76) boot: Partition Table:
      I (79) boot: ## Label            Usage          Type ST Offset   Length
      I (87) boot:  0 nvs              WiFi data        01 02 0000a000 00006000
      I (94) boot:  1 phy_init         RF data          01 01 00010000 00001000
      I (102) boot:  2 factory          factory app      00 00 00020000 00100000
      I (109) boot: End of partition table
      E (113) esp_image: image at 0x20000 has invalid magic byte
      W (120) esp_image: image at 0x20000 has invalid SPI mode 108
      W (126) esp_image: image at 0x20000 has invalid SPI size 11
      E (132) boot: Factory app partition is not bootable
      E (138) boot: No bootable app partitions in the partition table


.. _flash-encryption-status:

{IDF_TARGET_NAME} Flash Encryption Status
-----------------------------------------

1. Ensure that you have an {IDF_TARGET_NAME} device with default flash encryption eFuse settings as shown in :ref:`flash-encryption-efuse`.

To check if flash encryption on your {IDF_TARGET_NAME} device is enabled, do one of the following:

- flash the application example :example:`security/flash_encryption` onto your device. This application prints the ``{IDF_TARGET_CRYPT_CNT}`` eFuse value and if flash encryption is enabled or disabled.

- :doc:`Find the serial port name <../get-started/establish-serial-connection>` under which your {IDF_TARGET_NAME} device is connected, replace ``PORT`` with your port name in the following command, and run it:

  .. code-block:: bash

      espefuse.py -p PORT summary


.. _reading-writing-content:

Reading and Writing Data in Encrypted Flash
-------------------------------------------

{IDF_TARGET_NAME} application code can check if flash encryption is currently enabled by calling :cpp:func:`esp_flash_encryption_enabled`. Also, a device can identify the flash encryption mode by calling :cpp:func:`esp_get_flash_encryption_mode`.

Once flash encryption is enabled, be more careful with accessing flash contents from code.


Scope of Flash Encryption
^^^^^^^^^^^^^^^^^^^^^^^^^

Whenever the ``{IDF_TARGET_CRYPT_CNT}`` eFuse is set to a value with an odd number of bits, all flash content accessed via the MMU's flash cache is transparently decrypted. It includes:

- Executable application code in flash (IROM).
- All read-only data stored in flash (DROM).
- Any data accessed via :cpp:func:`spi_flash_mmap`.
- The firmware bootloader image when it is read by the ROM bootloader.

.. important::

    The MMU flash cache unconditionally decrypts all existing data. Data which is stored unencrypted in flash memory will also be "transparently decrypted" via the flash cache and will appear to software as random garbage.


Reading from Encrypted Flash
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To read data without using a flash cache MMU mapping, you can use the partition read function :cpp:func:`esp_partition_read`. This function will only decrypt data when it is read from an encrypted partition. Data read from unencrypted partitions will not be decrypted. In this way, software can access encrypted and non-encrypted flash in the same way.

You can also use the following SPI flash API functions:

- :cpp:func:`esp_flash_read` to read raw (encrypted) data which will not be decrypted
- :cpp:func:`esp_flash_read_encrypted` to read and decrypt data

The ROM function :cpp:func:`SPIRead` can read data without decryption, however, this function is not supported in esp-idf applications.

Data stored using the Non-Volatile Storage (NVS) API is always stored and read decrypted from the perspective of flash encryption. It is up to the library to provide encryption feature if required. Refer to :ref:`NVS Encryption <nvs_encryption>` for more details.


Writing to Encrypted Flash
^^^^^^^^^^^^^^^^^^^^^^^^^^

It is recommended to use the partition write function :cpp:func:`esp_partition_write`. This function will only encrypt data when it is written to an encrypted partition. Data written to unencrypted partitions will not be encrypted. In this way, software can access encrypted and non-encrypted flash in the same way.

You can also pre-encrypt and write data using the function :cpp:func:`esp_flash_write_encrypted`

Also, the following ROM function exist but not supported in esp-idf applications:

- ``esp_rom_spiflash_write_encrypted`` pre-encrypts and writes data to flash
- ``SPIWrite`` writes unencrypted data to flash

Since data is encrypted in blocks, the minimum write size for encrypted data is 16 bytes and the alignment is also 16 bytes.


.. _updating-encrypted-flash:

Updating Encrypted Flash
------------------------

.. _updating-encrypted-flash-ota:

OTA Updates
^^^^^^^^^^^

OTA updates to encrypted partitions will automatically write encrypted data if the function :cpp:func:`esp_partition_write` is used.

Before building the application image for OTA updating of an already encrypted device, enable the option :ref:`Enable flash encryption on boot <CONFIG_SECURE_FLASH_ENC_ENABLED>` in project configuration menu.

For general information about ESP-IDF OTA updates, please refer to :doc:`OTA <../api-reference/system/ota>`


.. _updating-encrypted-flash-serial:



Disabling Flash Encryption
--------------------------

If flash encryption was enabled accidentally, flashing of plaintext data will soft-brick the {IDF_TARGET_NAME}. The device will reboot continuously, printing the error ``flash read err, 1000``.

.. only:: esp32

  For flash encryption in Development mode, encryption can be disabled by burning the ``{IDF_TARGET_CRYPT_CNT}`` eFuse. It can only be done three times per chip by taking the following steps:

.. only:: esp32s2

  For flash encryption in Development mode, encryption can be disabled by burning the ``{IDF_TARGET_CRYPT_CNT}`` eFuse. It can only be done one time per chip by taking the following steps:

#. In :ref:`project-configuration-menu`, disable :ref:`Enable flash encryption on boot <CONFIG_SECURE_FLASH_ENC_ENABLED>`, then save and exit.
#. Open project configuration menu again and **double-check** that you have disabled this option! If this option is left enabled, the bootloader will immediately re-enable encryption when it boots.
#. With flash encryption disabled, build and flash the new bootloader and application by running ``idf.py flash``.
#. Use ``espefuse.py`` (in ``components/esptool_py/esptool``) to disable the ``{IDF_TARGET_CRYPT_CNT}`` by running:

  .. code-block:: bash

      espefuse.py burn_efuse {IDF_TARGET_CRYPT_CNT}

Reset the {IDF_TARGET_NAME}. Flash encryption will be disabled, and the bootloader will boot as usual.


Key Points About Flash Encryption
---------------------------------

.. list::

  :esp32: - Flash memory contents are encrypted using AES-256. The flash encryption key is stored in the ``BLOCK1`` eFuse internal to the chip and, by default, is protected from software access.

  :esp32: - The flash encryption algorithm is AES-256, where the key is "tweaked" with the offset address of each 32 byte block of flash. This means that every 32-byte block (two consecutive 16 byte AES blocks) is encrypted with a unique key derived from the flash encryption key.

  :esp32s2: - Flash memory contents are encrypted using XTS-AES-128 or XTS-AES-256. The flash encryption key is 256 bits and 512 bits respectively and stored one or two ``KEYN`` eFuses internal to the chip and, by default, is protected from software access.

  - Flash access is transparent via the flash cache mapping feature of {IDF_TARGET_NAME} - any flash regions which are mapped to the address space will be transparently decrypted when read.

    Some data partitions might need to remain unencrypted for ease of access or might require the use of flash-friendly update algorithms which are ineffective if the data is encrypted. NVS partitions for non-volatile storage cannot be encrypted since the NVS library is not directly compatible with flash encryption. For details, refer to :ref:`NVS Encryption <nvs_encryption>`.

  - If flash encryption might be used in future, the programmer must keep it in mind and take certain precautions when writing code that :ref:`uses encrypted flash <reading-writing-content>`.

  - If secure boot is enabled, re-flashing the bootloader of an encrypted device requires a "Re-flashable" secure boot digest (see :ref:`flash-encryption-and-secure-boot`).

    .. only:: esp32

        The firmware bootloader app binary ``bootloader.bin`` might become too large if both secure boot and flash encryption are enabled. See :ref:`secure-boot-bootloader-size`.

    .. important::

        Do not interrupt power to the {IDF_TARGET_NAME} while the first boot encryption pass is running. If power is interrupted, the flash contents will be corrupted and will require flashing with unencrypted data again. In this case, re-flashing will not count towards the flashing limit.


.. _flash-encryption-limitations:

Limitations of Flash Encryption
-------------------------------

Flash encryption protects firmware against unauthorised readout and modification. It is important to understand the limitations of the flash encryption feature:

.. list::
    - Flash encryption is only as strong as the key. For this reason, we recommend keys are generated on the device during first boot (default behaviour). If generating keys off-device, ensure proper procedure is followed and don't share the same key between all production devices.
    - Not all data is stored encrypted. If storing data on flash, check if the method you are using (library, API, etc.) supports flash encryption.
    - Flash encryption does not prevent an attacker from understanding the high-level layout of the flash. This is because the same AES key is used for every pair of adjacent 16 byte AES blocks. When these adjacent 16 byte blocks contain identical content (such as empty or padding areas), these blocks will encrypt to produce matching pairs of encrypted blocks. This may allow an attacker to make high-level comparisons between encrypted devices (i.e. to tell if two devices are probably running the same firmware version).
    :esp32: - For the same reason, an attacker can always tell when a pair of adjacent 16 byte blocks (32 byte aligned) contain two identical 16 byte sequences. Keep this in mind if storing sensitive data on the flash, design your flash storage so this doesn't happen (using a counter byte or some other non-identical value every 16 bytes is sufficient). :ref:`NVS Encryption <nvs_encryption>` deals with this and is suitable for many uses.
    :esp32: - Flash encryption alone may not prevent an attacker from modifying the firmware of the device. To prevent unauthorised firmware from running on the device, use flash encryption in combination with :doc:`Secure Boot <secure-boot-v2>`.
    :esp32s2: - Flash encryption alone may not prevent an attacker from modifying the firmware of the device. To prevent unauthorised firmware from running on the device, use flash encryption in combination with Secure Boot.

.. _flash-encryption-and-secure-boot:

Flash Encryption and Secure Boot
---------------------------------

It is recommended to use flash encryption in combination with Secure Boot. However, if Secure Boot is enabled, additional restrictions apply to device re-flashing:

- :ref:`updating-encrypted-flash-ota` are not restricted, provided that the new app is signed correctly with the Secure Boot signing key.

.. only:: esp32

    - :ref:`Plaintext serial flash updates <updating-encrypted-flash-serial>` are only possible if the :ref:`Re-flashable <CONFIG_SECURE_BOOTLOADER_MODE>` Secure Boot mode is selected and a Secure Boot key was pre-generated and burned to the {IDF_TARGET_NAME} (refer to :ref:`Secure Boot <secure-boot-reflashable>`). In such configuration, ``idf.py bootloader`` will produce a pre-digested bootloader and secure boot digest file for flashing at offset 0x0. When following the plaintext serial re-flashing steps it is necessary to re-flash this file before flashing other plaintext data.
    - :ref:`Re-flashing via Pregenerated Flash Encryption Key <pregenerated-flash-encryption-key>` is still possible, provided the bootloader is not re-flashed. Re-flashing the bootloader requires the same :ref:`Re-flashable <CONFIG_SECURE_BOOTLOADER_MODE>` option to be enabled in the Secure Boot config.


.. _flash-encryption-advanced-features:

Advanced Features
-----------------

The following section covers advanced features of flash encryption.

.. _encrypted-partition-flag:

Encrypted Partition Flag
^^^^^^^^^^^^^^^^^^^^^^^^

Some partitions are encrypted by default. Other partitions can be marked in the partition table description as requiring encryption by adding the flag ``encrypted`` to the partitions' flag field. As a result, data in these marked partitions will be treated as encrypted in the same manner as an app partition.

.. code-block:: bash

   # Name,   Type, SubType, Offset,  Size, Flags
   nvs,      data, nvs,     0x9000,  0x6000
   phy_init, data, phy,     0xf000,  0x1000
   factory,  app,  factory, 0x10000, 1M
   secret_data, 0x40, 0x01, 0x20000, 256K, encrypted

For details on partition table description, see :doc:`partition table <../api-guides/partition-tables>`.

Further information about encryption of partitions:

- Default partition tables do not include any encrypted data partitions.
- With flash encryption enabled, the ``app`` partition is always treated as encrypted and does not require marking.
- If flash encryption is not enabled, the flag "encrypted" has no effect.
- You can also consider protecting ``phy_init`` data from physical access, readout, or modification, by marking the optional ``phy`` partition with the flag ``encrypted``.
- The ``nvs`` partition cannot be encrypted, because the NVS library is not directly compatible with flash encryption.


.. _uart-bootloader-encryption:

Enabling UART Bootloader Encryption/Decryption
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

On the first boot, the flash encryption process burns by default the following eFuses:

.. only:: esp32

  - ``DISABLE_DL_ENCRYPT`` which disables flash encryption operation when running in UART bootloader boot mode.
  - ``DISABLE_DL_DECRYPT`` which disables transparent flash decryption when running in UART bootloader mode, even if the eFuse ``FLASH_CRYPT_CNT`` is set to enable it in normal operation.
  - ``DISABLE_DL_CACHE`` which disables the entire MMU flash cache when running in UART bootloader mode.

.. only:: esp32s2

  - ``EFUSE_DIS_DOWNLOAD_MANUAL_ENCRYPT`` flash encryption operation when running in UART bootloader boot mode.

However, before the first boot you can choose to keep any of these features enabled by burning only selected eFuses and write-protect the rest of eFuses with unset value 0. For example:

.. only:: esp32

  .. code-block:: bash

    espefuse.py --port PORT burn_efuse DISABLE_DL_DECRYPT
    espefuse.py --port PORT write_protect_efuse DISABLE_DL_ENCRYPT

  .. note::

      Set all appropriate bits before write-protecting!

      Write protection of all the three eFuses is controlled by one bit. It means that write-protecting one eFuse bit will inevitably write-protect all unset eFuse bits.

  Write protecting these eFuses to keep them unset is not currently very useful, as ``esptool.py`` does not support reading encrypted flash.

.. only:: esp32

  .. important::

      Leaving ``DISABLE_DL_DECRYPT`` unset (0) makes flash encryption useless.

      An attacker with physical access to the chip can use UART bootloader mode with custom stub code to read out the flash contents.


.. only:: esp32

  .. _setting-flash-crypt-config:

  Setting FLASH_CRYPT_CONFIG
  ^^^^^^^^^^^^^^^^^^^^^^^^^^

  The eFuse ``FLASH_CRYPT_CONFIG`` determines the number of bits in the flash encryption key which are "tweaked" with the block offset. For details, see :ref:`flash-encryption-algorithm`.

  On the first boot or the firmware bootloader, this value is set to the maximum ``0xF``.

  It is possible to burn this eFuse manually and write protect it before the first boot in order to select different tweak values. However, this is not recommended.

  It is strongly recommended to never write-protect ``FLASH_CRYPT_CONFIG`` when it is unset. Otherwise, its value will remain zero permanently, and no bits in the flash encryption key will be tweaked. As a result, the flash encryption algorithm will be equivalent to AES ECB mode.

JTAG Debugging
^^^^^^^^^^^^^^

By default, when Flash Encryption is enabled (in either Development or Release mode) then JTAG debugging is disabled via eFuse. The bootloader does this on first boot, at the same time it enables flash encryption.

See :ref:`jtag-debugging-security-features` for more information about using JTAG Debugging with Flash Encryption.

Technical Details
-----------------

The following sections provide some reference information about the operation of flash encryption.

.. only:: esp32

  .. _flash-encryption-algorithm:

  Flash Encryption Algorithm
  ^^^^^^^^^^^^^^^^^^^^^^^^^^

  - AES-256 operates on 16-byte blocks of data. The flash encryption engine encrypts and decrypts data in 32-byte blocks - two AES blocks in series.

  - The main flash encryption key is stored in the ``BLOCK1`` eFuse and, by default, is protected from further writes or software readout.

  - AES-256 key size is 256 bits (32 bytes) read from the ``BLOCK1`` eFuse. The hardware AES engine uses the key in reversed byte order as compared to the storage order in ``BLOCK1``.

    - If the ``CODING_SCHEME`` eFuse is set to ``0`` (default, "None" Coding Scheme) then the eFuse key block is 256 bits and the key is stored as-is (in reversed byte order).
    - If the ``CODING_SCHEME`` eFuse is set to ``1`` (3/4 Encoding) then the eFuse key block is 192 bits (in reversed byte order), so overall entropy is reduced. The hardware flash encryption still operates on a 256-bit key, after being read (and un-reversed), the key is extended as ``key = key[0:255] + key[64:127]``.

  - AES algorithm is used inverted in flash encryption, so the flash encryption "encrypt" operation is AES decrypt and the "decrypt" operation is AES encrypt. This is for performance reasons and does not alter the effeciency of the algorithm.

  - Each 32-byte block (two adjacent 16-byte AES blocks) is encrypted with a unique key. The key is derived from the main flash encryption key in ``BLOCK1``, XORed with the offset of this block in the flash (a "key tweak").

  - The specific tweak depends on the ``FLASH_CRYPT_CONFIG`` eFuse setting. This is a 4-bit eFuse where each bit enables XORing of a particular range of the key bits:

    - Bit 1, bits 0-66 of the key are XORed.
    - Bit 2, bits 67-131 of the key are XORed.
    - Bit 3, bits 132-194 of the key are XORed.
    - Bit 4, bits 195-256 of the key are XORed.

    It is recommended that ``FLASH_CRYPT_CONFIG`` is always left at the default value ``0xF``, so that all key bits are XORed with the block offset. For details, see :ref:`setting-flash-crypt-config`.

  - The high 19 bits of the block offset (bit 5 to bit 23) are XORed with the main flash encryption key. This range is chosen for two reasons: the maximum flash size is 16MB (24 bits), and each block is 32 bytes so the least significant 5 bits are always zero.

  - There is a particular mapping from each of the 19 block offset bits to the 256 bits of the flash encryption key to determine which bit is XORed with which. See the variable ``_FLASH_ENCRYPTION_TWEAK_PATTERN`` in the ``espsecure.py`` source code for complete mapping.

  - To see the full flash encryption algorithm implemented in Python, refer to the `_flash_encryption_operation()` function in the ``espsecure.py`` source code.

.. only:: esp32s2

  .. _flash-encryption-algorithm:

  Flash Encryption Algorithm
  ^^^^^^^^^^^^^^^^^^^^^^^^^^
  - {IDF_TARGET_NAME} use the XTS-AES block chiper mode with 256 bit or 512 bit key size for flash encryption.

  - XTS-AES is a block chiper mode specifically designed for disc encryption and addresses the weaknesses other potential modes (e.g. AES-CTR) have for this use case. A detailed description of the XTS-AES algorithm can be found in `IEEE Std 1619-2007 <https://ieeexplore.ieee.org/document/4493450>`_.

  - The flash encryption key is stored in one or two ``KEYN`` eFuses and, by default, is protected from further writes or software readout.

  - To see the full flash encryption algorithm implemented in Python, refer to the `_flash_encryption_operation()` function in the ``espsecure.py`` source code.
