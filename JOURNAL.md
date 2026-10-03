---
title: "EntropyX"
author: "your-name"
description: "A hardware security and entropy generation project built on NXP MCUXpresso and ARM Cortex-M33"
created_at: "2026-09-06"
---




# Sept 6: Initialised project and encountered debugger issues

Initialised and started the project and started with the blink test template, but however faced several issues, with it especially with link sever and and the gdb connection,LinkServer GDB server timed out during launch (Error 103: Remote connection closed) Spent several hours trying to find out what is the issue and I even purchased another MCUXN236 board, cuz I belived the board was fried or smth

<img width="523" height="275" alt="Screen Shot 2026-09-29 at 18 59 54 PM" src="https://github.com/user-attachments/assets/470f316b-00cc-4339-949b-3978a543e391" />

**Total time spent: 5.5 hours**

# Sept 8: Reinstalled tools and ordered a USB dongle

recieved the new board today, and did the unboxing, and as i nervously ran the build button and the debug button, only to be bombarded with the same issue and the persistant problems, i consulted nearly every possible information available on the internet to no avail, and then I understood that it was not the problem with my board and wondered at my reckless foolishness, so aftr that, i completely reinstalled the entire suite of apps and the MCUXpresso IDE along with the link server, later I also ran some terminal commands only to discover that the boar was not recognised by my mac on com8 properly, so as I consulted Claude, and it might be the issue with the newer thunderbolt port, so it recomended me to get a dongle, and guess what thats what i did .
<img width="1600" height="896" alt="WhatsApp Image 2026-09-17 at 9 43 17 PM" src="https://github.com/user-attachments/assets/a137cc58-178a-44b6-9d8d-827070803045" />

**Total time spent: 3.5 hours**

# Sept 9: Migrated build environment to Windows PC

Got my dongle off amazon today, but still f***ing faced the same issues, on mac, spent some more pointless hours trying to find out the issue, and was almost in the verge of giving up, and then thhought of trying it out on windows, so I got my brothers really old dell Celeron dual core pc, which ran horrifyingly slow, ( the boot itself took 5 mins) and after installed all the heavy applications which almost crashed my system and took forever just to install,I got back to my work and created my project and after days of trying finally got my bard to work

the reason i specifically wanted this exact board and not some off the shelf esp32 is because of the fact that it has inbuilt trustzone with its arm cortex m33 dual code chip, with onboard sha256 encryption, which was the whole point of my project.

But however super happy that i finally got it to work as i ditched my mac

**Total time spent: 6.5 hours**

# Sept 10: Created initial architecture and configured toolchain

Created the initial project using the NXP MCUXpresso IDE, created the files main.c, gy87.c, entropy_math.c, crypto_hw.c and the gy87.h, entropy_math.h, crypto_hw.h, and tried to configure it to the CMakeLists.txt so that it is properly configures  bt i kept making so many errors that i almost spent my entire morning on this one thing, and then also corrected it to the ARM GCC cross-compiler
<img width="1920" height="921" alt="656649749-9e1595f8-c789-4911-b250-baa40f942d81" src="https://github.com/user-attachments/assets/e4b525fb-d0e6-49fa-9f22-447ffad00629" />

**Total time spent: 3.5 hours**

# Sept 10: Reconfigured UART output and project templates

Used the led_blinky template to generate the project, which actively disables the physical UART (Serial) pins to save battery. The CPU did run  but the Serial Monitor was blank. so hard to completely restart my project with  and remade the CMakeLists.txt to use the hello_world board port, which forces pins P1_8 and P1_9 open for UART transmission.
<img width="422" height="451" alt="Screen Shot 2026-10-03 at 19 29 51 PM" src="https://github.com/user-attachments/assets/df8d07cb-5ebc-4ef0-acbc-c01536af9563" />


**Total time spent: 2.5 hours**

# Sept 11: Mapped hardware pinouts and wiring

Completed all the pin outs and the connections after though research with the board and the vaious components, hope it just works and i did all correctly
<img width="822" height="502" alt="Unknown-2" src="https://github.com/user-attachments/assets/0e63c955-a018-4a14-9da8-72007a936f13" />

**Total time spent: 1.5 hours**

# Sept 11: Researched SHA-256 and NIST key generation guidelines

spent time researching and learning aboiut the sha256 encrytiion and how it works and where it is used and how to randomly create the keys whch are unpredictable
also I spent time reading the nist documentations and the guidelines on it how works and how to keep it to the top standards and began my work with the main.c
<img width="1915" height="826" alt="Screen Shot 2026-10-03 at 19 27 16 PM" src="https://github.com/user-attachments/assets/fcf589b9-baf4-47d4-a8c4-02530cfe7105" />

**Total time spent: 3.2 hours**

# Sept 12: Configured cryptographic engine and purged dependencies

Done with the cryptography engine, the bare minimum
Enabling the Cryptography engine automatically triggered the NXP Kconfig system to inject a full FreeRTOS operating system and an AWS-IoT networking stack, crashing the build (portmacro.h missing so removed the unnessary dependencies and modified the hidden prj.conf file to forcefully disable middleware.freertos-kernel=n and iot_reference.logging=n afte which it worked fine, so part of my project turned out to work, but the builds and debug took a painfull lot of time and at times upto half an hour ,just to build the necessary project,
<img width="429" height="139" alt="Screen Shot 2026-10-03 at 19 28 07 PM" src="https://github.com/user-attachments/assets/244536ca-fbac-4715-a4ab-0cb8c8b3b51e" />

**Total time spent: 4.6 hours**

# Sept 12: Fixed hardware floating point linker dependencies

so now i had another issue The NIST algorithms require advanced math (log2f, sqrtf), causing "Undefined Reference" linker errors. so I had to aappend target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m) to CMake to manually link the ARM hardware math library. and did some more debugging
<img width="1920" height="921" alt="656649749-9e1595f8-c789-4911-b250-baa40f942d81" src="https://github.com/user-attachments/assets/2b708df0-b3f0-4032-82f2-9fd930779445" />

**Total time spent: 2.3 hours**

# Sept 13: Implemented I2C Radar Scanner and relocated bus pins

Wrote an I2C Radar Scanner from  total scratch( with no help) to ping every address from 0x01 to 0x7F to map the hardware network.but however radar scanner found absolutely nothing on pins P1_16/P1_17 (Arduino headers). whose reason which i later got to know because those pins lacked physical pull-up resistors hence the time was slow.
so after some research and reading the datasheets i migrated the entire physical bus to Flexcomm 2 (P4_0 / P4_1) on the MikroBUS header, which possesses factory-soldered 2.2 kΩ pull-up resistors.
<img width="622" height="456" alt="Screen Shot 2026-10-03 at 19 34 06 PM" src="https://github.com/user-attachments/assets/0a8e6961-790c-48cb-a99e-80d7b7f89080" />

**Total time spent: 5.5 hours**

# Sept 14: Resolved magnetometer I2C bypass and QMC6308 address mapping

now, another hurdle MPU6050 responded at 0x68, but the Magnetometer was not responding, i tried a couple of different approches but i had no idea what to do , and later with my brothers advice since the magnetometer is wired behind the MPU6050.i sent a command to 0x37 = 0x02 on the MPU6050 to enable "I2C Bypass Mode", opening the gate to the compass.
but once this was fixed, i discovered that my standard GY-87 code failed to read the compass. which i later figured out was because of my QMC6308 chip (Address 0x2C) instead of the standard HMC5883L (Address 0x1E). , but once these bugs were finally solved I had to had to do the actual math, and to remove the deterministic biases as explained in the readme.md

so this is my next objective , i habe to extract the quantum noise, remove deterministic physical biases, and mathematically prove the randomness using US Government standards.(NIST) gotten started, super excited for it

**Total time spent: 6.2 hours**

# Sept 16: Programmed Min-Entropy equation and engineered gravity filter

Solved the inability to print floating-point numbers (a limitation of the newlib-nano compiler flag) by splitting floats into integers: %d.%03d.Programmed the NIST SP 800-90B Smooth Min-Entropy equation, but however soon after that 1G gravity vector created a massive static bias on the Z-axis of the accelerometer. The NIST tests failed instantly.
so in order to remove all the static gravity Engineered a First-Order Backward Difference Filter (ΔS(t)=S(t)−S(t−1). By subtracting the previous reading from the current reading to make it free from biases

**Total time spent: 3 hours**

# Sept 16: Built Toeplitz matrix extractor and SHA-256 key conditioning

Implemented a 256×1024 binary Toeplitz matrix universal hash extractor in entropy_math.c. matrix generation via a 32-bit primitive Galois LFSR polynomial with seed 0xA5C39541.
Linked on-chip hardware-accelerated SHA-256 compression to condition extracted bits into a final 256-bit symmetric key.

**Total time spent: 3.5 hours**

# Sept 17: Added live SAC verification, SH1106 display driver, and DWT timing

made aure that the toeplitz matrix works correctly without any issues and also added live Strict Avalanche Criterion (SAC) verification in main.c and hamming verification
added a 3000ms delay between each key generation, so that the sha256 key will be visible,wrote ssd1306.c to fully support the 1.3" SH1106 controller with the (0x02, 0x10).
verified that it works and made sure that it is working correctly and the key generation takes place
Enabled the Cortex-M33 Data Watchpoint and Trace (DWT) cycle counter in CoreDebug to support sub-nanosecond timing, which is highly necessary 
<img width="241" height="441" alt="Screen Shot 2026-10-03 at 19 34 44 PM" src="https://github.com/user-attachments/assets/5b00f49f-56dc-46c0-b9be-22859ae65fe0" />

**Total time spent: 2.5 hours**

# Sept 19: Finalized hardware-software boundary and cryptographic verification

The most time-consuming and challenging aspect of this project was not writing the cryptography algorithms, but crossing the Hardware Software Boundary fixing the I2C electrical signaling, navigating NXP's complex Kconfig build system, and managing linker dependencies on a small device.
completed the major part of the project today and the cryptographic code works fine for now, i have ensure that the key is only output , when it meets all the strict criterion nd the nist standards, else it shall not work and will not provide the keys.
<img width="1600" height="900" alt="Unknown" src="https://github.com/user-attachments/assets/536e7159-8ee8-4c85-b8bf-3de4423e72b0" />

**Total time spent: 5 hours**

# Sept 20: Built Web Serial application for live encryption testing

Wrote a complete HTML Web Serial application from scratch to connect directly to the FRDM board's USB COM port all directly from your browser and read the oncoming data and testing the live encrytion with the keys that are given. had to fix a bug to increase the time at ehich it will monitor continousy and also ensure that the avalanche effect was working properly , since at first i did not even start up
<img width="1340" height="639" alt="Screenshot 2026-09-18 182001" src="https://github.com/user-attachments/assets/8af52961-e19d-419c-b6de-744c170ad709" />

so this is how my project was made with over 60 hours of gruelling effort

**Total time spent: 2 hours**

# Sept 25: Created video demonstrations and final documentation

For recording the video demonstrations, completing the readme.md, the ppt, the guides, the journal.md
https://drive.google.com/file/d/1pVwUifi--L539-WKeUtXxB8Pm3j9NlTL/view?usp=share_link
**Total time spent: 3.5 hours**
