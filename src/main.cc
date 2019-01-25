/* ************************************************************************
 * Copyright 2018 Advanced Micro Devices, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ************************************************************************/

#include "commands.h"
#include "sevcore.h"    // for measurement_t. //todo remove?
#include "utilities.h"
#include <stdio.h>
#include <string>
#include <cstring>  // memcpy

static std::string DisplayBuildInfo()
{
    sev_user_data_status status_data;
    int cmd_ret = ERROR_UNSUPPORTED;

    std::string api_major_ver = "API_Major: xxx";
    std::string api_minor_ver = "API_Minor: xxx";
    std::string build_id_ver  = "BuildID: xxx";

    cmd_ret = gSEVDevice.platform_status(&status_data);
    if (cmd_ret != STATUS_SUCCESS)
        return "";

    char MajorBuf[4], MinorBuf[4], BuildIDBuf[4];          // +1 for Null char
    sprintf(MajorBuf, "%d", status_data.api_major);
    sprintf(MinorBuf, "%d", status_data.api_minor);
    sprintf(BuildIDBuf, "%d", status_data.build);
    api_major_ver.replace(11, 3, MajorBuf);
    api_minor_ver.replace(11, 3, MinorBuf);
    build_id_ver.replace(9, 3, BuildIDBuf);

    return api_major_ver + ", " + api_minor_ver + ", " + build_id_ver;
}

static void sysinfo()
{
    std::string cmd = "";
    std::string output = "";

    printf("-------------------------System Info-------------------------");

    // Exec bash commands to get info on user's platform and append to the output string
    cmd = "echo -n 'Hostname: '; hostname";
    ExecuteSystemCommand(cmd, &output);
    cmd = "echo -n 'BIOS Version: '; dmidecode -s bios-version";
    ExecuteSystemCommand(cmd, &output);
    cmd = "echo -n 'BIOS Release Date: '; dmidecode -s bios-release-date";
    ExecuteSystemCommand(cmd, &output);
    cmd = "echo -n 'SMT/Multi-Threading Status Per Socket: \n'; lscpu | grep -E \"^CPU\\(s\\):|Thread\\(s\\) per core|Core\\(s\\) per socket|Socket\\(s\\)\"";
    ExecuteSystemCommand(cmd, &output);
    cmd = "echo -n 'Processor Frequency (all sockets): \n'; dmidecode -s processor-frequency";
    ExecuteSystemCommand(cmd, &output);
    cmd = "echo -n 'Operating System: '; cat /etc/os-release | grep \"PRETTY_NAME=\" | sed 's/.*=//'";        // cat /etc/issue
    ExecuteSystemCommand(cmd, &output);
    cmd = "echo -n 'Kernel Version: '; uname -r";
    ExecuteSystemCommand(cmd, &output);
    cmd = "echo -n 'Git Commit #: '; cat \"../.git/refs/heads/master\"";
    ExecuteSystemCommand(cmd, &output);

    // Print results of all ExecuteSystemCommand calls
    printf("\n%s", output.c_str());

    std::string BuildInfo = DisplayBuildInfo();
    printf("Firmware Version: %s\n", BuildInfo.c_str());

    printf("-------------------------------------------------------------\n\n");
}


char helpArray[] = "The following commands are supported:\n" \
                   "(Please see the readme file for more detailed information)\n" \
                   "  factory_reset\n" \
                   "  platform_status\n" \
                   "  pek_gen\n" \
                   "  pek_csr\n" \
                   "  pdh_gen\n" \
                   "  pdh_cert_export\n" \
                   "  pek_cert_import\n" \
                   "  get_id\n" \
                   "  calc_measurement\n" \
                   "      Input params (all in ascii-encoded hex bytes):\n" \
                   "          uint8_t  meas_ctx\n" \
                   "          uint8_t  api_major\n" \
                   "          uint8_t  api_minor\n" \
                   "          uint8_t  build_id\n" \
                   "          uint32_t policy\n" \
                   "          uint32_t digest\n" \
                   "          uint8_t mnonce[128/8]\n" \
                   "          uint8_t gctx_tik[128/8]\n" \
                   "  set_self_owed\n" \
                   "  set_externally_owned\n" \
                   ;

uint32_t map_arg_to_cmd(std::string arg)
{
    uint32_t ret = 0;

    if(strcmp(arg.c_str(), "factory_reset") == 0)
        ret = CMD_FACTORY_RESET;
    else if(strcmp(arg.c_str(), "platform_status") == 0)
        ret = CMD_PLATFORM_STATUS;
    else if(strcmp(arg.c_str(), "pek_gen") == 0)
        ret = CMD_PEK_GEN;
    else if(strcmp(arg.c_str(), "pek_csr") == 0)
        ret = CMD_PEK_CSR;
    else if(strcmp(arg.c_str(), "pdh_gen") == 0)
        ret = CMD_PDH_GEN;
    else if(strcmp(arg.c_str(), "pdh_cert_export") == 0)
        ret = CMD_PDH_CERT_EXPORT;
    else if(strcmp(arg.c_str(), "pek_cert_import") == 0)
        ret = CMD_PEK_CERT_IMPORT;
    else if(strcmp(arg.c_str(), "get_id") == 0)
        ret = CMD_GET_ID;
    else if(strcmp(arg.c_str(), "calc_measurement") == 0)
        ret = CMD_CALC_MEASUREMENT;
    else if(strcmp(arg.c_str(), "set_self_owned") == 0)
        ret = CMD_SET_SELF_OWNED;
    else if(strcmp(arg.c_str(), "set_externally_owned") == 0)
        ret = CMD_SET_EXT_OWNED;
    else
        ret = CMD_MAX;

    return ret;
}


int main(int argc, char** argv)
{
    Command cmd;
    SEV_ERROR_CODE cmd_ret = ERROR_UNSUPPORTED;

    sysinfo();  // Display system info

    printf("You have entered %i arguments\n", argc);
    if(argc <= 1) {     // User didnt enter any args
        printf("%s\n", helpArray);
        return 0;
    }

    // Second param is input Command
    printf("Command: %s\n\n", argv[1]);
    uint32_t int_arg = map_arg_to_cmd(argv[1]);
    switch (int_arg) {
        case CMD_FACTORY_RESET: {       // PLATFORM_RESET
            cmd_ret = cmd.factory_reset();
            break;
        }
        case CMD_PLATFORM_STATUS: {
            cmd_ret = cmd.platform_status();
            break;
        }
        case CMD_PEK_GEN: {
            cmd_ret = cmd.pek_gen();
            break;
        }
        case CMD_PEK_CSR: {
            cmd_ret = cmd.pek_csr();
            break;
        }
        case CMD_PDH_GEN: {
            cmd_ret = cmd.pdh_gen();
            break;
        }
        case CMD_PDH_CERT_EXPORT: {
            cmd_ret = cmd.pdh_cert_export();
            break;
        }
        case CMD_PEK_CERT_IMPORT: {
            cmd_ret = cmd.pek_cert_import();
            break;
        }
        case CMD_GET_ID: {
            cmd_ret = cmd.get_id();
            break;
        }
        case CMD_CALC_MEASUREMENT: {
            if(argc != 10) {
                printf("Error: Expecting 10 args for CALC_MEASUREMENT\n");
                break;
            }
            measurement_t user_data;
            user_data.meas_ctx  = (uint8_t)strtol(argv[2], NULL, 16);
            user_data.api_major = (uint8_t)strtol(argv[3], NULL, 16);
            user_data.api_minor = (uint8_t)strtol(argv[4], NULL, 16);
            user_data.build_id  = (uint8_t)strtol(argv[5], NULL, 16);
            user_data.policy    = (uint32_t)strtol(argv[6], NULL, 16);
            StrToArray(std::string(argv[7]), (uint8_t*)&user_data.digest, sizeof(user_data.digest));
            StrToArray(std::string(argv[8]), (uint8_t*)&user_data.mnonce, sizeof(user_data.mnonce));
            StrToArray(std::string(argv[9]), (uint8_t*)&user_data.tik,    sizeof(user_data.tik));
            cmd_ret = cmd.calc_measurement(&user_data);
            break;
        }
        case CMD_SET_SELF_OWNED: {
            cmd_ret = cmd.set_self_owned();
            break;
        }
        case CMD_SET_EXT_OWNED: {
            cmd_ret = cmd.set_externally_owned();
            break;
        }
        default: {
            printf("%s\n", helpArray);
            break;
        }
    }

    if(cmd_ret == STATUS_SUCCESS)
        printf("\nCommand Successful\n");
    else
        printf("\nCommand Unsuccessful: 0x%02x\n", cmd_ret);

    return 0;
}