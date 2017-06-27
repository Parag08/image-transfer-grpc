/*
 *
 * Copyright 2015, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>

//opencv
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/security/server_credentials.h>
#include "image.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;
using imagetransfer::Image;
using imagetransfer::ImageTransfer;


using namespace cv;

class RouteGuideImpl final : public ImageTransfer::Service {
 public:
  Status ImageChat(ServerContext* context,
                   ServerReaderWriter<Image, Image>* stream) override {
    std::vector<Image> received_notes;
    Image note;
    while (stream->Read(&note)) {
      std::cout << "message:"  << note.name()<<note.imagearray().size() << std::endl;
      /*for (const RouteNote& n : received_notes) {
          stream->Write(n);
      }*/
      stream->Write(note);
      //received_notes.push_back(note);
      int height = note.height();
      int width = note.width();
      int dimension  = note.depth();
      Mat  img = Mat::zeros(10,10, CV_8UC3);
      int ptr=0;
      for (int i = 0;  i < img.rows; i++) {
          for (int j = 0; j < img.cols; j++) {                                     
              img.at<cv::Vec3b>(i,j) = cv::Vec3b(note.imagearray()[ptr+ 0],note.imagearray()[ptr+1],note.imagearray()[ptr+2]);
              ptr=ptr+3;
          }
       }
       std::cout<<"mat:"<<img<<std::endl;
    }
    return Status::OK;
  }

};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  RouteGuideImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}

int main(int argc, char** argv) {
  // Expect only arg: --db_path=path/to/route_guide_db.json.
  RunServer();

  return 0;
}
