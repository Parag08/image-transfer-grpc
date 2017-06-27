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

#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <thread>
#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>
#include "image.grpc.pb.h"

//openCV

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace cv;

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;
using imagetransfer::Image;
using imagetransfer::ImageTransfer;


Image MakeImage(const std::string& message) {
  Image new_image;
  Mat frame;        // creates just the header parts
  frame = imread(message, CV_LOAD_IMAGE_COLOR);
  new_image.set_height(frame.rows);
  new_image.set_width(frame.cols);
  new_image.set_depth(frame.dims);
  //std::cout << "mat:" <<  frame << std::endl; 
  Mat mat = (frame.reshape(0,1));
  std::vector<int> array;
  if (mat.isContinuous()) {
     array.assign(mat.datastart, mat.dataend);
  } else {
     for (int i = 0; i < mat.rows; ++i) {
         array.insert(array.end(), mat.ptr<uchar>(i), mat.ptr<uchar>(i)+mat.cols);
      }
  }
  for(auto const& value: array) {
    new_image.add_imagearray(value);
  }
  new_image.set_name(message);
  return new_image;
}

class ImageTransferClient {
 public:
  ImageTransferClient(std::shared_ptr<Channel> channel)
      : stub_(ImageTransfer::NewStub(channel)) {
  }
  void ImageChat() {
    ClientContext context;

    std::shared_ptr<ClientReaderWriter<Image, Image> > stream(
        stub_->ImageChat(&context));

    std::thread writer([stream]() {
      std::vector<Image> notes{
        MakeImage("10x10.png"),
        MakeImage("50x50.png"),
        MakeImage("test_image.png")
      };
      for (const Image& note : notes) {
        std::cout << "Sending message " << note.name() << std::endl;
        stream->Write(note);
      }
      stream->WritesDone();
    });

    Image server_note;
    while (stream->Read(&server_note)) {
      std::cout << "Got message " << server_note.name() << std::endl;
    }
    writer.join();
    Status status = stream->Finish();
    if (!status.ok()) {
      std::cout << "RouteChat rpc failed." << std::endl;
    }
  }
 private:
   std::unique_ptr<ImageTransfer::Stub> stub_;
};

ImageTransferClient ImageClient(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

int main(int argc, char** argv) {
  ImageClient.ImageChat();
  std::cout<<"done" << std::endl;
  return 0;
}
