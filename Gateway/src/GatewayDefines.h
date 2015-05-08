/*
 * GatewayDefines.h
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 tomoaki@tomy-tech.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *  Created on: 2015/05/01
 *    Modified:
 *      Author: Tomoaki YAMAGUCHI
 *     Version: 0.0.0
 */

#ifndef GATEWAYDEFINES_H_
#define GATEWAYDEFINES_H_

#define GATEWAY_VERSION "(Ver 1.2.0)"
#define GATEWAY_TYPE "AsyncTomyGateway"

#define BROKER      "Broker"
#define GREEN_BROKER "\x1b[0m\x1b[32mBroker\x1b[0m\x1b[37m"
#define GATEWAY     "Gateway"
#define CLIENT      "Client"
#define LEFTARROW   "<---"
#define RIGHTARROW  "--->"

#define FORMAT      "%s   %-14s%-8s%-44s%s\n"
#define FORMAT1     "%s   %-14s%-8s%-26s%s\n"
#define FORMAT2     "\n%s   %-14s%-8s%-26s%s\n"

#define RED_FORMAT1      "%s   \x1b[0m\x1b[31m%-14s%-8s%-44s\x1b[0m\x1b[37m%s\n"
#define RED_FORMAT2    "\n%s   \x1b[0m\x1b[31m%-14s%-8s%-26s\x1b[0m\x1b[37m%s\n"

#define GREEN_FORMAT     "%s   \x1b[0m\x1b[32m%-14s%-8s%-44s\x1b[0m\x1b[37m%s\n"
#define GREEN_FORMAT1    "%s   \x1b[0m\x1b[32m%-14s%-8s%-26s\x1b[0m\x1b[37m%s\n"
#define GREEN_FORMAT2  "\n%s   \x1b[0m\x1b[32m%-14s%-8s%-26s\x1b[0m\x1b[37m%s\n"

#define YELLOW_FORMAT1   "%s   \x1b[0m\x1b[33m%-14s%-8s%-26s\x1b[0m\x1b[37m%s\n"
#define YELLOW_FORMAT2 "\n%s   \x1b[0m\x1b[33m%-14s%-8s%-26s\x1b[0m\x1b[37m%s\n"

#define BLUE_FORMAT      "%s   \x1b[0m\x1b[34m%-14s%-8s%-44s\x1b[0m\x1b[37m%s\n"
#define BLUE_FORMAT1     "%s   \x1b[0m\x1b[34m%-14s%-8s%-26s\x1b[0m\x1b[37m%s\n"
#define BLUE_FORMAT2   "\n%s   \x1b[0m\x1b[34m%-14s%-8s%-26s\x1b[0m\x1b[37m%s\n"

#define CYAN_FORMAT1     "%s   \x1b[0m\x1b[36m%-14s%-8s%-26s\x1b[0m\x1b[37m%s\n"
#define SYAN_FORMAT2   "\n%s   \x1b[0m\x1b[36m%-14s%-8s%-26s\x1b[0m\x1b[37m%s\n"

/*===========================================
 *   Gateway Control Constants
 ===========================================*/

#define BROKER_HOST_NAME  "localhost"
#define BROKER_PORT       "1883"


#define KEEP_ALIVE_TIME   900    // 900 sec = 15 min

#define TIMEOUT_PERIOD     10    //  10 sec = 10 sec

#define SEND_UNIXTIME_TIME 30    // 30sec after KEEP_ALIVE_TIME



#define MAX_CLIENT_NODES  500

/*==========================================================
 *           Light Indicators
 ===========================================================*/
#define LIGHT_INDICATOR_GREEN   4    // RPi connector 16
#define LIGHT_INDICATOR_RED     5    // RPi connector 18
#define LIGHT_INDICATOR_BLUE    6    // RPi connector 22

#endif /* GATEWAYDEFINES_H_ */
