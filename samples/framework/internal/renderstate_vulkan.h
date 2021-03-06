//----------------------------------------------------------------------------//
//                                                                            //
// ozz-animation is hosted at http://github.com/guillaumeblanc/ozz-animation  //
// and distributed under the MIT License (MIT).                               //
//                                                                            //
// Copyright (c) 2016 Fabio Polimeni                                          //
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

#ifndef OZZ_SAMPLES_FRAMEWORK_RENDERSTATE_VULKAN_H_
#define OZZ_SAMPLES_FRAMEWORK_RENDERSTATE_VULKAN_H_

#ifndef OZZ_INCLUDE_PRIVATE_HEADER
#error "This header is private, it cannot be included from public headers."
#endif  // OZZ_INCLUDE_PRIVATE_HEADER

namespace ozz {
	namespace sample {

		namespace internal {
			class ContextVulkan;
		}

		namespace vk {

			class RenderState {

			protected:

				internal::ContextVulkan* renderContext;

				RenderState() { }

			public:

				virtual ~RenderState() { }

				// Gives a chance to create the render resources
				virtual bool onInitResources(internal::ContextVulkan* context);

				// Gives a chance to release all the owned resources
				virtual void onReleaseResources();

				// This function is called when command buffers are recorded,
				// between vkCmdBeginRenderPass() and vkCmdEndRenderPass.
				virtual bool onRegisterRenderPass(size_t commandIndex) = 0;

				// Because we can have multiple render passes,
				// this callback specify when the render context
				// has finished to register them all.
				virtual void onRenderPassesComplete() = 0;

				// This is called when the swap chain get a resize update,
				// it give the chance to re-initialize the graphics pipeline.
				virtual bool onSwapChainChange() = 0;

				// This check is executed every frame, and if true,
				// the render-context render-pass has to be reset
				// and re-recorded.
				virtual bool isDirty() = 0;
			};

		}
	}
}

#endif //OZZ_SAMPLES_FRAMEWORK_RENDERSTATE_VULKAN_H_