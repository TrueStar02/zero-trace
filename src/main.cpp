// SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause)
/* Copyright (c) 2020 Facebook */

#include "basic_types.h"
#include "event_defs.h"
#include "hook_point.h"
#include "iotrace.skel.h"
#include "iotracer.h"
#include "log.h"
#include "qemu_uprobe.skel.h"
#include <argp.h>
#include <assert.h>
#include <bpf/libbpf.h>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <getopt.h>
#include <memory>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <utility>
#include "config.h"

unsigned long long Request::request_id = 0;
unsigned long long Request::done_count = 0;
unsigned long long Request::avg_d2c = 0;
unsigned long long Request::avg_q2c = 0;
unsigned long long Request::avg_q2d = 0;
unsigned long long Request::avg_readpage = 0;
unsigned long long Request::avg_offcpu = 0;
unsigned long long Request::avg_time = 0;
unsigned long long Request::bio_cnt=0;

Event::MemoryPool Event::memory_pool_;
IOTracer *tracer = nullptr;

static void sig_handler(int signo) {
  if (signo == SIGINT || signo == SIGTERM) {
    printf("Detaching program\n");
    exit(0);
  }
}

void parse_cmd_args(int argc, char **argv, TraceConfig &config,
                    std::string &output_file) {
  /*
          Parse the arguments
  */
  long long opt;
  int enable_qemu_tracing = 0;
  int asGuest = 0;
  int asHost = 0;

  while ((opt = getopt(argc, argv, "p:t:d:c:f:D:o:w:n:h:q:G:H")) != -1) {
    switch (opt) {
    case 'p':
      config.pid = atoi(optarg);
      break;
    case 't':
      config.tid = atoi(optarg);
      break;
    case 'd':
      config.device_path = std::string(optarg);
      break;
    case 'c':
      config.cgroup_path = std::string(optarg);
      break;
    case 'f':
      config.file_path = std::string(optarg);
      // printf("file path: %s\n", config.file_path.c_str());
      break;
    case 'D':
      config.directory_path = std::string(optarg);
      break;
    case 'o':
      output_file = std::string(optarg);
      break;
    case 'w':
      config.time_threshold = atof(optarg);
      break;
    case 'n':
      config.task_name = std::string(optarg);
      break;
    case 'q':
      enable_qemu_tracing = 1;
      break;
    case 'G':
      config.asGuest = 1;
      break;
    case 'H':
      config.asHost = 1;
      break;
    default:
      fprintf(stderr,
              "Usage: %s [-p pid] [-t tid] [-d dev path] [-c cgroup path] [-f file path] [-D "
              "directory path] [-o output] [-w time threshold] [-G run as guest] [-H run as host] \n",
              argv[0]);
      exit(EXIT_FAILURE);
    }
  }
}

int main(int argc, char **argv) {
  /* Cleaner handling of Ctrl-C */
  signal(SIGINT, sig_handler);
  signal(SIGTERM, sig_handler);

  TraceConfig config;

  std::string output_file;
  parse_cmd_args(argc, argv, config, output_file);
  std::unique_ptr<DoneRequestHandler> logHandler;
  #ifdef GRAFANA
  logHandler =
      std::unique_ptr<DoneRequestHandler>(new GrafanaClientLogHandler(output_file));
  #else
  logHandler =
      std::unique_ptr<DoneRequestHandler>(new FileLogHandler(output_file));
  #endif
  tracer = new IOTracer(std::move(logHandler), std::move(config));
  tracer->start();
  return 0;
}