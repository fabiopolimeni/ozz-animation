//----------------------------------------------------------------------------//
//                                                                            //
// ozz-animation is hosted at http://github.com/guillaumeblanc/ozz-animation  //
// and distributed under the MIT License (MIT).                               //
//                                                                            //
// Copyright (c) 2015 Guillaume Blanc                                         //
//                                                                            //
// Permission is hereby granted, free of charge, to any person obtaining a    //
// copy of this software and associated documentation files (the "Software"), //
// to deal in the Software without restriction, including without limitation  //
// the rights to use, copy, modify, merge, publish, distribute, sublicense,   //
// and/or sell copies of the Software, and to permit persons to whom the      //
// Software is furnished to do so, subject to the following conditions:       //
//                                                                            //
// The above copyright notice and this permission notice shall be included in //
// all copies or substantial portions of the Software.                        //
//                                                                            //
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR //
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   //
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    //
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER //
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    //
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        //
// DEALINGS IN THE SOFTWARE.                                                  //
//                                                                            //
//----------------------------------------------------------------------------//

#ifndef OZZ_SAMPLES_FRAMEWORK_SHOOTER_H_
#define OZZ_SAMPLES_FRAMEWORK_SHOOTER_H_

#include "ozz/base/platform.h"

namespace ozz {
namespace sample {

// Interface for screen shot and video shooter
class Shooter
{

public:
	 
  Shooter() { }
  virtual ~Shooter() { }

  // Resize notification, used to resize memory buffers.
  virtual void Resize(int _width, int _height) = 0;

  // Updates shooter (output captured buffers to memory).
  virtual bool Update() = 0;

  // Captures current (FRONT or BACK) _buffer to memory.
  virtual bool Capture(int _buffer) = 0;

};
}  // sample
}  // ozz
#endif  // OZZ_SAMPLES_FRAMEWORK_INTERNAL_SHOOTER_H_
