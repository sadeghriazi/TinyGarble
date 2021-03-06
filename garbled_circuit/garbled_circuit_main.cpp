/*
 This file is part of JustGarble.

 JustGarble is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 JustGarble is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with JustGarble.  If not, see <http://www.gnu.org/licenses/>.

 */
/*
 This file is part of TinyGarble. It is modified version of JustGarble
 under GNU license.

 TinyGarble is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TinyGarble is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TinyGarble.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <cstdlib>
#include <ctime>

#include "scd/scd.h"
#include "garbled_circuit/garbled_circuit.h"
#include "tcpip/tcpip.h"
#include "util/util.h"
#include "util/tinygarble_config.h"
#include "util/log.h"

namespace po = boost::program_options;
using std::string;
using std::vector;

int main(int argc, char* argv[]) {

  LogInitial(argc, argv);
  srand(time(0));  // srand(1);
  SrandSSE(time(0));  // SrandSSE(1111);

  int port;
  string scd_file_address;
  string server_ip;
  string init_str;
  string input_str;
  uint64_t clock_cycles;
  string output_mask;
  int output_mode;
  bool disable_OT = false;
  boost::format fmter(
      "Evaluate Netlist, TinyGarble version %1%.%2%.%3%.\nAllowed options");
  fmter % TinyGarble_VERSION_MAJOR % TinyGarble_VERSION_MINOR
      % TinyGarble_VERSION_PATCH;
  po::options_description desc(fmter.str());
  desc.add_options()  //
  ("help,h", "produce help message")  //
  ("alice,a", "Run as Alice (server).")  //
  ("bob,b", "Run as Bob (client).")  //
  ("scd_file,i",
   po::value<string>(&scd_file_address)->default_value(
       "../scd/netlists/hamming_32bit_1cc.scd"),
   "Simple circuit description (.scd) file address.")  //
  ("port,p", po::value<int>(&port)->default_value(1234), "socket port")  //
  ("server_ip,s", po::value<string>(&server_ip)->default_value("127.0.0.1"),
   "Server's (Alice's) IP, required when running as Bob.")  //
  ("init", po::value<string>(&init_str),
   "Hexadecimal init for initializing DFFs, if not provided, it will be 0.")  //
  ("input", po::value<string>(&input_str),
   "Hexadecimal input, if not provided, it will be 0.")  //
  ("clock_cycles", po::value<uint64_t>(&clock_cycles)->default_value(1),
   "Number of clock cycles to evaluate the circuit.")  //
  ("dump_directory", po::value<string>(&dump_prefix)->default_value(""),
   "Directory for dumping memory hex files.")  //
  ("disable_OT", "Disable Oblivious Transfer (OT) for transferring labels. "
   "WARNING: OT is crucial for GC security.")  //
  ("output_mask", po::value<string>(&output_mask)->default_value("0"),
   "Hexadecimal mask for output. 0 indicates that output belongs to Alice, "
   "and 1 belongs to Bob ")  //
  ("output_mode", po::value<int>(&output_mode)->default_value(0),
   "0: normal, 1:separated by clock 2:last clock.");

  po::variables_map vm;
  try {
    po::parsed_options parsed = po::command_line_parser(argc, argv).options(
        desc).allow_unregistered().run();
    po::store(parsed, vm);
    if (vm.count("help")) {
      std::cout << desc << endl;
      return SUCCESS;
    }
    po::notify(vm);
  } catch (po::error& e) {
    LOG(ERROR) << "ERROR: " << e.what() << endl << endl;
    std::cout << desc << endl;
    return FAILURE;
  }

  if (vm.count("disable_OT")) {
    disable_OT = true;
    LOG(INFO) << "OT is disabled. WARNING:OT is crucial for GC security."
              << endl;
  }

  if (vm.count("alice") == 0 && vm.count("bob") == 0) {
    LOG(ERROR) << "One of --alice or --bob mode flag should be used." << endl
               << endl;
    std::cout << desc << endl;
    return FAILURE;
  }

  if (vm.count("alice")) {
    // open the socket
    int connfd;
    if ((connfd = ServerInit(port)) == -1) {
      LOG(ERROR) << "Cannot open the socket in port " << port << endl;
      return FAILURE;
    }
    LOG(INFO) << "Open Alice's server on port: " << port << endl;

    string output_str;
    uint64_t delta_time = RDTSC;
    CHECK(
        GarbleStr(scd_file_address, init_str, input_str, clock_cycles,
                  output_mask, output_mode, disable_OT, &output_str, connfd));
    delta_time = RDTSC - delta_time;

    LOG(INFO) << "Alice's output = " << output_str << endl;
    LOG(INFO) << "Total Alice time (cc) = " << delta_time << endl;
    std::cout << output_str << endl;

    ServerClose(connfd);
  } else if (vm.count("bob")) {

    if (vm.count("server_ip")) {
      server_ip = vm["server_ip"].as<string>();
    } else {
      LOG(ERROR) << "Server IP should be specified, when running as Bob."
                 << endl << endl;
      std::cout << desc << endl;
      return -1;
    }

    // open socket, connect to server
    int connfd;
    if ((connfd = ClientInit(server_ip.c_str(), port)) == -1) {
      LOG(ERROR) << "Cannot connect to " << server_ip << ":" << port << endl;
      return FAILURE;
    }
    LOG(INFO) << "Connect Bob's client to Alice's server on " << server_ip
              << ":" << port << endl;

    string output_str;
    uint64_t delta_time = RDTSC;
    CHECK(
        EvaluateStr(scd_file_address, init_str, input_str, clock_cycles,
                    output_mask, output_mode, disable_OT, &output_str, connfd));
    delta_time = RDTSC - delta_time;

    LOG(INFO) << "Bob's output = " << output_str << endl;
    LOG(INFO) << "Total Bob time (cc) = " << delta_time << endl;
    std::cout << output_str << endl;

    ClientClose(connfd);
  }

  LogFinish();
  return SUCCESS;
}

