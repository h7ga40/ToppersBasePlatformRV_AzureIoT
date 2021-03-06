// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

//This file pulls in OS-specific header files to allow compilation of socket_async.c under
// most OS's except for Windows.

// For Linux systems
#include <sys/types.h>
//#include <lwip/sockets.h>
#include <sys/select.h>
#include <sys/errno.h>
#include <unistd.h>
//#include <netdb.h>

