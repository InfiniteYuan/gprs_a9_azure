# 将用于C的Azure IoT设备客户端SDK移植到新设备

C SDK和库：

用ANSI C（C99）编写，避免编译器扩展，以最大限度地提高代码可移植性和广泛的平台兼容性。
公开平台抽象层以隔离OS依赖性（线程和互斥机制，通信协议，例如HTTP）。 有关我们的抽象层的更多信息，请参阅我们的移植指南。
在存储库中，您将找到用于在Linux，Windows和微控制器平台上编译和运行C设备客户端SDK的指令和构建工具（有关为C编译设备客户端的更多信息，请参阅上面的链接）。

如果您正在考虑将设备客户端SDK for C移植到新平台，请查看移植指南文档。

# 如何将Azure IoT C SDK移植到其他平台

本文档的目的是提供有关如何将C共享实用程序库移植到不支持开箱即用的平台的指导。 C共享实用程序库由IoTHub客户端SDK和EventHub客户端SDK等C SDK使用。

该文档未涵盖任何特定平台的细节。

## 参考

- Specifications

tickcounter adapter specification
agenttime adapter specification
threadapi and sleep adapter specification
platform adapter specification
tlsio specification
xio specification
lock adapter specification

- Header files
tickcounter.h
threadapi.h
xio.h
tlsio.h
socketio.h
lock.h
platform.h

## 概述

C共享实用程序库是用C99编写的，目的是为大多数平台提供可移植性。 但是，若干组件依赖于特定于平台的资源以实现所需的功能。 因此，移植C共享实用程序库是通过移植属于库的PAL组件（适配器）来完成的。 以下是组件的概述：

有几个必须提供适配器的必备组件：

- tickcounter 实现：这为SDK提供了一个适配器，用于获取以ms为单位表示的tick计数器。精度不必是毫秒，而是提供给SDK的值必须以毫秒表示。

- agenttime 实现：这为C时间管理功能提供了SDK适配器，如时间，difftime等。由于不同平台处理时间的方式存在很大差异，因此需要这样做。

- sleep 实现，提供与设备无关的睡眠功能。

- 用于执行全局init和de-init的 platform 实现。

- 允许SDK通过TLS进行通信的 tlsio 实现。物联网中心不支持不安全的通信。

此外，还有两个可选组件 threadapi 和 lock，它们允许SDK在专用线程上与IoT Hub进行通信。另一个可选组件是 socketio 适配器，它与某些类型的 tlsio 适配器实现一起使用。

在SDK的适配器目录和源目录下可以找到几个现有的适配器。如果这些适配器中的任何一个适用于您的设备，只需将该文件直接包含在项目中即可。

## tickcounter适配器
Tickcounter行为在tickcounter规范中指定。

要开始使用，请复制此版本的tickcounter.c文件并对其进行修改以适合您的设备。

如果内存大小是个问题，则tickcounter规范包含可能适合您的优化建议。

## agenttime适配器
agenttime适配器在agenttime适配器规范中指定，并提供与平台无关的时间函数。

对于大多数平台/操作系统，您可以在构建中包含标准的agenttime.c文件。 这个适配器只调用C函数time，difftime，ctime等。

如果此文件不适用于您的实现，请复制它并适当地修改它。

Azure IoT SDK仅需要get_time和get_difftime函数。 此适配器中的其他函数 - get_gmtime，get_mktime和get_ctime - 已弃用，可能会被省略或不起作用。

## sleep适配器
睡眠适配器是一个单一功能，提供与设备无关的线程休眠。 与其他适配器不同，它没有自己的头文件。 相反，它的声明包含在threadapi.h文件中，同时包含可选的threadapi适配器的声明，其实现通常包含在关联的threadapi.c文件中。

但是，与threadapi.h中的其他函数不同，SDK需要ThreadAPI_Sleep，并且必须始终可以正常运行。

睡眠适配器的规范可在threadapi和sleep adapter规范中找到。

## platform适配器
平台适配器为平台执行一次初始化和取消初始化，并为SDK提供适当的TLSIO。

平台适配器规范提供了编写平台适配器的完整说明。

要开始创建平台适配器，请复制此Windows platform.c文件并根据需要进行修改。

## threadapi和lock适配器
必须存在用于SDK编译的threadapi和lock适配器，但它们的功能是可选的。他们的规范文档（见下文）详细说明了如果不需要线程功能，每个空函数应该做什么。

这些组件允许SDK在专用线程内与IoT Hub通信。由于需要单独的堆栈，使用专用线程在内存消耗方面有一些成本，这可能使专用线程难以在具有很少空闲内存的设备上使用。

专用线程的优点是当网络不可用时，所有当前的tlsio适配器在尝试连接到其IoT Hub时可能会重复阻塞一段时间，并且拥有专用于Azure IoT SDK的线程将允许其他设备功能在SDK被阻止时保持响应。

SDK的未来版本可能会消除这种潜在的阻塞行为，但是目前，必须响应的设备需要为SDK使用专用线程，这需要实现ThreadApi和Lock适配器。

以下是threadapi和锁适配器的规范：

threadapi和睡眠适配器规范
锁适配器规范
这些规范解释了如何在不需要线程时创建null lock和threadapi适配器。

要开始创建threadapi和锁适配器，请复制这些Windows适配器文件并对其进行适当修改：

threadapi.c
lock.c

## tlsio适配器概述
tlsio适配器为SDK提供与Azure IoT Hub的安全TLS通信。

Tlsio适配器通过xio向SDK公开其功能，xio是这里定义的通用C语言bit-in-bit-out接口。

通过适配器的xio_create函数创建tlsio适配器实例，并且在创建tlsio实例时，配置参数const void * io_create_parameters必须是专用类型TLSIO_CONFIG。

为新设备实现tlsio适配器是通过选择最符合您需求的现有tlsio适配器（也可能是一个socketio适配器）并相应地修改它来完成的。

两种风格的tlsio
tlsio适配器有两种可能的设计模式：直接和链接。在直接样式中，tlsio适配器拥有一个TCP套接字，用于直接与远程主机执行TLS通信。在链式样式中，tlsio适配器不拥有TCP套接字，并且不直接与远程主机通信。相反，它仍然执行所有TLS杂务，如加密，解密和协商，但它通过另一种xio风格的适配器间接与远程主机通信。

直接样式适配器需要更少的代码并避免额外的内存缓冲区复制，因此它们更适合微控制器。 Arduino和ESP32的tlsio适配器都是直接类型。链式风格适配器更耗资源，但它们提供更多灵活性。适用于Windows，Linux和Mac的tlsio适配器都是链式样式适配器。

TLS实现的选择可能决定了tlsio的风格。例如，针对ESP32的Arduino和Espressif的OpenSSL实现的TLS实现只能直接使用它们自己的内部TCP套接字，因此它们只能用作直接样式tlsio的一部分。相比之下，OpenSSL的完整官方版本可以使用任何一种方式。

## socketio适配器概述
为了连接到互联网，链接的tlsio必须在某个时候通过包含TCP套接字的xio适配器进行通信。 在Azure IoT SDK中，管理TCP套接字的xio适配器称为socketio适配器。

如果需要，可以将多个xio组件链接在一起。 下面的图表说明了直接tlsio，链式tlsio和带有基于xio的http代理组件的链式tlsio之间的区别：

仅显示xio_http_proxy组件以说明xio功能。 其详细信息超出了本文档的范围。

## tlsio适配器实现
tlsio以外的适配器易于实现，即使对于没有经验的开发人员也是如此。 然而，tlsio适配器非常复杂，编写tlsio适配器只是经验丰富的开发人员的任务，他们很乐意在没有指令的情况下设置包含目录和外部库。

有关tlsio实现的最新信息包含在tlsio规范中。

所有现有的tlsio适配器都使用不属于Azure IoT SDK的TLS库。 有关库使用的说明，请参阅特定TSL库的文档，例如设置包含目录和链接库文件。

## 现有的直接tlsio实现
现有两个直接适配器实现：

ESP32的tlsio_openssl_compact
tlsio_arduino为Arduino
在这两个中，ESP32的tlsio_openssl_compact可能是复制用于重用的更好的候选者，因为它更可能类似于更新的设备，并且它与tlsio规范一起编写。

ESP32的tlsio_openssl_compact使用这两个文件抽象其操作系统依赖项：

socket_async.c
dns_async.c
建议所有直接tlsio实现都遵循这种模式。

socket_async.c和dns_async.c文件只需更改包含os特定标头的“socket_async_os.h”文件的内容，就可以在不更改大多数套接字实现的情况下重复使用。

## 现有的链式tlsio实现
链式适配器实现包括：

适用于Windows，Linux和Mac的tlsio_openssl
tlsio_schannel仅适用于Windows
tlsio_wolfssl用于嵌入式设备
tlsio_cyclonessl用于嵌入式设备
tlsio_mbedtls为mbed

## 链式tlsio适配器的现有socketio实现
socketio适配器没有规范，因此有必要复制现有的行为，而socketio_berkeley是复制的最佳选择。但是，请注意所有现有的socketio适配器（包括socketio_berkeley）在socketio_destroy期间清除其挂起的io列表，这是不正确的。应该在socketio_close期间清除挂起的io列表。

适用于Linux的socketio_berkeley，可轻松适应任何Berkeley套接字
适用于Windows的socketio_win32
socketio_mbed为mbed
tlsio_cyclonessl_socket用于cyclonessl

## 添加设备支持存储库
新支持的设备可能使用各种可能的构建系统，因此不能规定单个标准源树。 我们建议将ESP32的支持存储库作为创建新设备支持存储库的模型。