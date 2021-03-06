// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is auto-generated from
// gpu/command_buffer/build_gles2_cmd_buffer.py
// It's formatted by clang-format using chromium coding style:
//    clang-format -i -style=chromium filename
// DO NOT EDIT!

#ifndef GPU_COMMAND_BUFFER_COMMON_GLES2_CMD_IDS_AUTOGEN_H_
#define GPU_COMMAND_BUFFER_COMMON_GLES2_CMD_IDS_AUTOGEN_H_

#define GLES2_COMMAND_LIST(OP)                                           \
  OP(ActiveTexture)                                            /* 256 */ \
  OP(AttachShader)                                             /* 257 */ \
  OP(BindAttribLocationBucket)                                 /* 258 */ \
  OP(BindBuffer)                                               /* 259 */ \
  OP(BindBufferBase)                                           /* 260 */ \
  OP(BindBufferRange)                                          /* 261 */ \
  OP(BindFramebuffer)                                          /* 262 */ \
  OP(BindRenderbuffer)                                         /* 263 */ \
  OP(BindSampler)                                              /* 264 */ \
  OP(BindTexture)                                              /* 265 */ \
  OP(BindTransformFeedback)                                    /* 266 */ \
  OP(BlendColor)                                               /* 267 */ \
  OP(BlendEquation)                                            /* 268 */ \
  OP(BlendEquationSeparate)                                    /* 269 */ \
  OP(BlendFunc)                                                /* 270 */ \
  OP(BlendFuncSeparate)                                        /* 271 */ \
  OP(BufferData)                                               /* 272 */ \
  OP(BufferSubData)                                            /* 273 */ \
  OP(CheckFramebufferStatus)                                   /* 274 */ \
  OP(Clear)                                                    /* 275 */ \
  OP(ClearBufferfi)                                            /* 276 */ \
  OP(ClearBufferfvImmediate)                                   /* 277 */ \
  OP(ClearBufferivImmediate)                                   /* 278 */ \
  OP(ClearBufferuivImmediate)                                  /* 279 */ \
  OP(ClearColor)                                               /* 280 */ \
  OP(ClearDepthf)                                              /* 281 */ \
  OP(ClearStencil)                                             /* 282 */ \
  OP(ClientWaitSync)                                           /* 283 */ \
  OP(ColorMask)                                                /* 284 */ \
  OP(CompileShader)                                            /* 285 */ \
  OP(CompressedTexImage2DBucket)                               /* 286 */ \
  OP(CompressedTexImage2D)                                     /* 287 */ \
  OP(CompressedTexSubImage2DBucket)                            /* 288 */ \
  OP(CompressedTexSubImage2D)                                  /* 289 */ \
  OP(CompressedTexImage3DBucket)                               /* 290 */ \
  OP(CompressedTexImage3D)                                     /* 291 */ \
  OP(CompressedTexSubImage3DBucket)                            /* 292 */ \
  OP(CompressedTexSubImage3D)                                  /* 293 */ \
  OP(CopyBufferSubData)                                        /* 294 */ \
  OP(CopyTexImage2D)                                           /* 295 */ \
  OP(CopyTexSubImage2D)                                        /* 296 */ \
  OP(CopyTexSubImage3D)                                        /* 297 */ \
  OP(CreateProgram)                                            /* 298 */ \
  OP(CreateShader)                                             /* 299 */ \
  OP(CullFace)                                                 /* 300 */ \
  OP(DeleteBuffersImmediate)                                   /* 301 */ \
  OP(DeleteFramebuffersImmediate)                              /* 302 */ \
  OP(DeleteProgram)                                            /* 303 */ \
  OP(DeleteRenderbuffersImmediate)                             /* 304 */ \
  OP(DeleteSamplersImmediate)                                  /* 305 */ \
  OP(DeleteSync)                                               /* 306 */ \
  OP(DeleteShader)                                             /* 307 */ \
  OP(DeleteTexturesImmediate)                                  /* 308 */ \
  OP(DeleteTransformFeedbacksImmediate)                        /* 309 */ \
  OP(DepthFunc)                                                /* 310 */ \
  OP(DepthMask)                                                /* 311 */ \
  OP(DepthRangef)                                              /* 312 */ \
  OP(DetachShader)                                             /* 313 */ \
  OP(Disable)                                                  /* 314 */ \
  OP(DisableVertexAttribArray)                                 /* 315 */ \
  OP(DrawArrays)                                               /* 316 */ \
  OP(DrawElements)                                             /* 317 */ \
  OP(Enable)                                                   /* 318 */ \
  OP(EnableVertexAttribArray)                                  /* 319 */ \
  OP(FenceSync)                                                /* 320 */ \
  OP(Finish)                                                   /* 321 */ \
  OP(Flush)                                                    /* 322 */ \
  OP(FramebufferRenderbuffer)                                  /* 323 */ \
  OP(FramebufferTexture2D)                                     /* 324 */ \
  OP(FramebufferTextureLayer)                                  /* 325 */ \
  OP(FrontFace)                                                /* 326 */ \
  OP(GenBuffersImmediate)                                      /* 327 */ \
  OP(GenerateMipmap)                                           /* 328 */ \
  OP(GenFramebuffersImmediate)                                 /* 329 */ \
  OP(GenRenderbuffersImmediate)                                /* 330 */ \
  OP(GenSamplersImmediate)                                     /* 331 */ \
  OP(GenTexturesImmediate)                                     /* 332 */ \
  OP(GenTransformFeedbacksImmediate)                           /* 333 */ \
  OP(GetActiveAttrib)                                          /* 334 */ \
  OP(GetActiveUniform)                                         /* 335 */ \
  OP(GetActiveUniformBlockiv)                                  /* 336 */ \
  OP(GetActiveUniformBlockName)                                /* 337 */ \
  OP(GetActiveUniformsiv)                                      /* 338 */ \
  OP(GetAttachedShaders)                                       /* 339 */ \
  OP(GetAttribLocation)                                        /* 340 */ \
  OP(GetBooleanv)                                              /* 341 */ \
  OP(GetBufferParameteri64v)                                   /* 342 */ \
  OP(GetBufferParameteriv)                                     /* 343 */ \
  OP(GetError)                                                 /* 344 */ \
  OP(GetFloatv)                                                /* 345 */ \
  OP(GetFragDataLocation)                                      /* 346 */ \
  OP(GetFramebufferAttachmentParameteriv)                      /* 347 */ \
  OP(GetInteger64v)                                            /* 348 */ \
  OP(GetIntegeri_v)                                            /* 349 */ \
  OP(GetInteger64i_v)                                          /* 350 */ \
  OP(GetIntegerv)                                              /* 351 */ \
  OP(GetInternalformativ)                                      /* 352 */ \
  OP(GetProgramiv)                                             /* 353 */ \
  OP(GetProgramInfoLog)                                        /* 354 */ \
  OP(GetRenderbufferParameteriv)                               /* 355 */ \
  OP(GetSamplerParameterfv)                                    /* 356 */ \
  OP(GetSamplerParameteriv)                                    /* 357 */ \
  OP(GetShaderiv)                                              /* 358 */ \
  OP(GetShaderInfoLog)                                         /* 359 */ \
  OP(GetShaderPrecisionFormat)                                 /* 360 */ \
  OP(GetShaderSource)                                          /* 361 */ \
  OP(GetString)                                                /* 362 */ \
  OP(GetSynciv)                                                /* 363 */ \
  OP(GetTexParameterfv)                                        /* 364 */ \
  OP(GetTexParameteriv)                                        /* 365 */ \
  OP(GetTransformFeedbackVarying)                              /* 366 */ \
  OP(GetUniformBlockIndex)                                     /* 367 */ \
  OP(GetUniformfv)                                             /* 368 */ \
  OP(GetUniformiv)                                             /* 369 */ \
  OP(GetUniformuiv)                                            /* 370 */ \
  OP(GetUniformIndices)                                        /* 371 */ \
  OP(GetUniformLocation)                                       /* 372 */ \
  OP(GetVertexAttribfv)                                        /* 373 */ \
  OP(GetVertexAttribiv)                                        /* 374 */ \
  OP(GetVertexAttribIiv)                                       /* 375 */ \
  OP(GetVertexAttribIuiv)                                      /* 376 */ \
  OP(GetVertexAttribPointerv)                                  /* 377 */ \
  OP(Hint)                                                     /* 378 */ \
  OP(InvalidateFramebufferImmediate)                           /* 379 */ \
  OP(InvalidateSubFramebufferImmediate)                        /* 380 */ \
  OP(IsBuffer)                                                 /* 381 */ \
  OP(IsEnabled)                                                /* 382 */ \
  OP(IsFramebuffer)                                            /* 383 */ \
  OP(IsProgram)                                                /* 384 */ \
  OP(IsRenderbuffer)                                           /* 385 */ \
  OP(IsSampler)                                                /* 386 */ \
  OP(IsShader)                                                 /* 387 */ \
  OP(IsSync)                                                   /* 388 */ \
  OP(IsTexture)                                                /* 389 */ \
  OP(IsTransformFeedback)                                      /* 390 */ \
  OP(LineWidth)                                                /* 391 */ \
  OP(LinkProgram)                                              /* 392 */ \
  OP(PauseTransformFeedback)                                   /* 393 */ \
  OP(PixelStorei)                                              /* 394 */ \
  OP(PolygonOffset)                                            /* 395 */ \
  OP(ReadBuffer)                                               /* 396 */ \
  OP(ReadPixels)                                               /* 397 */ \
  OP(ReleaseShaderCompiler)                                    /* 398 */ \
  OP(RenderbufferStorage)                                      /* 399 */ \
  OP(ResumeTransformFeedback)                                  /* 400 */ \
  OP(SampleCoverage)                                           /* 401 */ \
  OP(SamplerParameterf)                                        /* 402 */ \
  OP(SamplerParameterfvImmediate)                              /* 403 */ \
  OP(SamplerParameteri)                                        /* 404 */ \
  OP(SamplerParameterivImmediate)                              /* 405 */ \
  OP(Scissor)                                                  /* 406 */ \
  OP(ShaderBinary)                                             /* 407 */ \
  OP(ShaderSourceBucket)                                       /* 408 */ \
  OP(MultiDrawBeginCHROMIUM)                                   /* 409 */ \
  OP(MultiDrawEndCHROMIUM)                                     /* 410 */ \
  OP(MultiDrawArraysCHROMIUM)                                  /* 411 */ \
  OP(MultiDrawArraysInstancedCHROMIUM)                         /* 412 */ \
  OP(MultiDrawArraysInstancedBaseInstanceCHROMIUM)             /* 413 */ \
  OP(MultiDrawElementsCHROMIUM)                                /* 414 */ \
  OP(MultiDrawElementsInstancedCHROMIUM)                       /* 415 */ \
  OP(MultiDrawElementsInstancedBaseVertexBaseInstanceCHROMIUM) /* 416 */ \
  OP(StencilFunc)                                              /* 417 */ \
  OP(StencilFuncSeparate)                                      /* 418 */ \
  OP(StencilMask)                                              /* 419 */ \
  OP(StencilMaskSeparate)                                      /* 420 */ \
  OP(StencilOp)                                                /* 421 */ \
  OP(StencilOpSeparate)                                        /* 422 */ \
  OP(TexImage2D)                                               /* 423 */ \
  OP(TexImage3D)                                               /* 424 */ \
  OP(TexParameterf)                                            /* 425 */ \
  OP(TexParameterfvImmediate)                                  /* 426 */ \
  OP(TexParameteri)                                            /* 427 */ \
  OP(TexParameterivImmediate)                                  /* 428 */ \
  OP(TexStorage3D)                                             /* 429 */ \
  OP(TexSubImage2D)                                            /* 430 */ \
  OP(TexSubImage3D)                                            /* 431 */ \
  OP(TransformFeedbackVaryingsBucket)                          /* 432 */ \
  OP(Uniform1f)                                                /* 433 */ \
  OP(Uniform1fvImmediate)                                      /* 434 */ \
  OP(Uniform1i)                                                /* 435 */ \
  OP(Uniform1ivImmediate)                                      /* 436 */ \
  OP(Uniform1ui)                                               /* 437 */ \
  OP(Uniform1uivImmediate)                                     /* 438 */ \
  OP(Uniform2f)                                                /* 439 */ \
  OP(Uniform2fvImmediate)                                      /* 440 */ \
  OP(Uniform2i)                                                /* 441 */ \
  OP(Uniform2ivImmediate)                                      /* 442 */ \
  OP(Uniform2ui)                                               /* 443 */ \
  OP(Uniform2uivImmediate)                                     /* 444 */ \
  OP(Uniform3f)                                                /* 445 */ \
  OP(Uniform3fvImmediate)                                      /* 446 */ \
  OP(Uniform3i)                                                /* 447 */ \
  OP(Uniform3ivImmediate)                                      /* 448 */ \
  OP(Uniform3ui)                                               /* 449 */ \
  OP(Uniform3uivImmediate)                                     /* 450 */ \
  OP(Uniform4f)                                                /* 451 */ \
  OP(Uniform4fvImmediate)                                      /* 452 */ \
  OP(Uniform4i)                                                /* 453 */ \
  OP(Uniform4ivImmediate)                                      /* 454 */ \
  OP(Uniform4ui)                                               /* 455 */ \
  OP(Uniform4uivImmediate)                                     /* 456 */ \
  OP(UniformBlockBinding)                                      /* 457 */ \
  OP(UniformMatrix2fvImmediate)                                /* 458 */ \
  OP(UniformMatrix2x3fvImmediate)                              /* 459 */ \
  OP(UniformMatrix2x4fvImmediate)                              /* 460 */ \
  OP(UniformMatrix3fvImmediate)                                /* 461 */ \
  OP(UniformMatrix3x2fvImmediate)                              /* 462 */ \
  OP(UniformMatrix3x4fvImmediate)                              /* 463 */ \
  OP(UniformMatrix4fvImmediate)                                /* 464 */ \
  OP(UniformMatrix4x2fvImmediate)                              /* 465 */ \
  OP(UniformMatrix4x3fvImmediate)                              /* 466 */ \
  OP(UseProgram)                                               /* 467 */ \
  OP(ValidateProgram)                                          /* 468 */ \
  OP(VertexAttrib1f)                                           /* 469 */ \
  OP(VertexAttrib1fvImmediate)                                 /* 470 */ \
  OP(VertexAttrib2f)                                           /* 471 */ \
  OP(VertexAttrib2fvImmediate)                                 /* 472 */ \
  OP(VertexAttrib3f)                                           /* 473 */ \
  OP(VertexAttrib3fvImmediate)                                 /* 474 */ \
  OP(VertexAttrib4f)                                           /* 475 */ \
  OP(VertexAttrib4fvImmediate)                                 /* 476 */ \
  OP(VertexAttribI4i)                                          /* 477 */ \
  OP(VertexAttribI4ivImmediate)                                /* 478 */ \
  OP(VertexAttribI4ui)                                         /* 479 */ \
  OP(VertexAttribI4uivImmediate)                               /* 480 */ \
  OP(VertexAttribIPointer)                                     /* 481 */ \
  OP(VertexAttribPointer)                                      /* 482 */ \
  OP(Viewport)                                                 /* 483 */ \
  OP(WaitSync)                                                 /* 484 */ \
  OP(BlitFramebufferCHROMIUM)                                  /* 485 */ \
  OP(RenderbufferStorageMultisampleCHROMIUM)                   /* 486 */ \
  OP(RenderbufferStorageMultisampleAdvancedAMD)                /* 487 */ \
  OP(RenderbufferStorageMultisampleEXT)                        /* 488 */ \
  OP(FramebufferTexture2DMultisampleEXT)                       /* 489 */ \
  OP(TexStorage2DEXT)                                          /* 490 */ \
  OP(GenQueriesEXTImmediate)                                   /* 491 */ \
  OP(DeleteQueriesEXTImmediate)                                /* 492 */ \
  OP(QueryCounterEXT)                                          /* 493 */ \
  OP(BeginQueryEXT)                                            /* 494 */ \
  OP(BeginTransformFeedback)                                   /* 495 */ \
  OP(EndQueryEXT)                                              /* 496 */ \
  OP(EndTransformFeedback)                                     /* 497 */ \
  OP(SetDisjointValueSyncCHROMIUM)                             /* 498 */ \
  OP(InsertEventMarkerEXT)                                     /* 499 */ \
  OP(PushGroupMarkerEXT)                                       /* 500 */ \
  OP(PopGroupMarkerEXT)                                        /* 501 */ \
  OP(GenVertexArraysOESImmediate)                              /* 502 */ \
  OP(DeleteVertexArraysOESImmediate)                           /* 503 */ \
  OP(IsVertexArrayOES)                                         /* 504 */ \
  OP(BindVertexArrayOES)                                       /* 505 */ \
  OP(FramebufferParameteri)                                    /* 506 */ \
  OP(BindImageTexture)                                         /* 507 */ \
  OP(DispatchCompute)                                          /* 508 */ \
  OP(DispatchComputeIndirect)                                  /* 509 */ \
  OP(DrawArraysIndirect)                                       /* 510 */ \
  OP(DrawElementsIndirect)                                     /* 511 */ \
  OP(GetProgramInterfaceiv)                                    /* 512 */ \
  OP(GetProgramResourceIndex)                                  /* 513 */ \
  OP(GetProgramResourceName)                                   /* 514 */ \
  OP(GetProgramResourceiv)                                     /* 515 */ \
  OP(GetProgramResourceLocation)                               /* 516 */ \
  OP(MemoryBarrierEXT)                                         /* 517 */ \
  OP(MemoryBarrierByRegion)                                    /* 518 */ \
  OP(SwapBuffers)                                              /* 519 */ \
  OP(GetMaxValueInBufferCHROMIUM)                              /* 520 */ \
  OP(EnableFeatureCHROMIUM)                                    /* 521 */ \
  OP(MapBufferRange)                                           /* 522 */ \
  OP(UnmapBuffer)                                              /* 523 */ \
  OP(FlushMappedBufferRange)                                   /* 524 */ \
  OP(ResizeCHROMIUM)                                           /* 525 */ \
  OP(GetRequestableExtensionsCHROMIUM)                         /* 526 */ \
  OP(RequestExtensionCHROMIUM)                                 /* 527 */ \
  OP(GetProgramInfoCHROMIUM)                                   /* 528 */ \
  OP(GetUniformBlocksCHROMIUM)                                 /* 529 */ \
  OP(GetTransformFeedbackVaryingsCHROMIUM)                     /* 530 */ \
  OP(GetUniformsES3CHROMIUM)                                   /* 531 */ \
  OP(DescheduleUntilFinishedCHROMIUM)                          /* 532 */ \
  OP(GetTranslatedShaderSourceANGLE)                           /* 533 */ \
  OP(PostSubBufferCHROMIUM)                                    /* 534 */ \
  OP(CopyTextureCHROMIUM)                                      /* 535 */ \
  OP(CopySubTextureCHROMIUM)                                   /* 536 */ \
  OP(DrawArraysInstancedANGLE)                                 /* 537 */ \
  OP(DrawArraysInstancedBaseInstanceANGLE)                     /* 538 */ \
  OP(DrawElementsInstancedANGLE)                               /* 539 */ \
  OP(DrawElementsInstancedBaseVertexBaseInstanceANGLE)         /* 540 */ \
  OP(VertexAttribDivisorANGLE)                                 /* 541 */ \
  OP(ProduceTextureDirectCHROMIUMImmediate)                    /* 542 */ \
  OP(CreateAndConsumeTextureINTERNALImmediate)                 /* 543 */ \
  OP(BindUniformLocationCHROMIUMBucket)                        /* 544 */ \
  OP(BindTexImage2DCHROMIUM)                                   /* 545 */ \
  OP(BindTexImage2DWithInternalformatCHROMIUM)                 /* 546 */ \
  OP(ReleaseTexImage2DCHROMIUM)                                /* 547 */ \
  OP(TraceBeginCHROMIUM)                                       /* 548 */ \
  OP(TraceEndCHROMIUM)                                         /* 549 */ \
  OP(DiscardFramebufferEXTImmediate)                           /* 550 */ \
  OP(LoseContextCHROMIUM)                                      /* 551 */ \
  OP(UnpremultiplyAndDitherCopyCHROMIUM)                       /* 552 */ \
  OP(DrawBuffersEXTImmediate)                                  /* 553 */ \
  OP(DiscardBackbufferCHROMIUM)                                /* 554 */ \
  OP(ScheduleOverlayPlaneCHROMIUM)                             /* 555 */ \
  OP(ScheduleCALayerSharedStateCHROMIUM)                       /* 556 */ \
  OP(ScheduleCALayerCHROMIUM)                                  /* 557 */ \
  OP(ScheduleCALayerInUseQueryCHROMIUMImmediate)               /* 558 */ \
  OP(CommitOverlayPlanesCHROMIUM)                              /* 559 */ \
  OP(FlushDriverCachesCHROMIUM)                                /* 560 */ \
  OP(ScheduleDCLayerCHROMIUM)                                  /* 561 */ \
  OP(SetActiveURLCHROMIUM)                                     /* 562 */ \
  OP(ContextVisibilityHintCHROMIUM)                            /* 563 */ \
  OP(CoverageModulationCHROMIUM)                               /* 564 */ \
  OP(BlendBarrierKHR)                                          /* 565 */ \
  OP(BindFragDataLocationIndexedEXTBucket)                     /* 566 */ \
  OP(BindFragDataLocationEXTBucket)                            /* 567 */ \
  OP(GetFragDataIndexEXT)                                      /* 568 */ \
  OP(UniformMatrix4fvStreamTextureMatrixCHROMIUMImmediate)     /* 569 */ \
  OP(OverlayPromotionHintCHROMIUM)                             /* 570 */ \
  OP(SwapBuffersWithBoundsCHROMIUMImmediate)                   /* 571 */ \
  OP(SetDrawRectangleCHROMIUM)                                 /* 572 */ \
  OP(SetEnableDCLayersCHROMIUM)                                /* 573 */ \
  OP(InitializeDiscardableTextureCHROMIUM)                     /* 574 */ \
  OP(UnlockDiscardableTextureCHROMIUM)                         /* 575 */ \
  OP(LockDiscardableTextureCHROMIUM)                           /* 576 */ \
  OP(TexStorage2DImageCHROMIUM)                                /* 577 */ \
  OP(SetColorSpaceMetadataCHROMIUM)                            /* 578 */ \
  OP(WindowRectanglesEXTImmediate)                             /* 579 */ \
  OP(CreateGpuFenceINTERNAL)                                   /* 580 */ \
  OP(WaitGpuFenceCHROMIUM)                                     /* 581 */ \
  OP(DestroyGpuFenceCHROMIUM)                                  /* 582 */ \
  OP(SetReadbackBufferShadowAllocationINTERNAL)                /* 583 */ \
  OP(FramebufferTextureMultiviewOVR)                           /* 584 */ \
  OP(MaxShaderCompilerThreadsKHR)                              /* 585 */ \
  OP(CreateAndTexStorage2DSharedImageINTERNALImmediate)        /* 586 */ \
  OP(BeginSharedImageAccessDirectCHROMIUM)                     /* 587 */ \
  OP(EndSharedImageAccessDirectCHROMIUM)                       /* 588 */ \
  OP(BeginBatchReadAccessSharedImageCHROMIUM)                  /* 589 */ \
  OP(EndBatchReadAccessSharedImageCHROMIUM)                    /* 590 */

enum CommandId {
  kOneBeforeStartPoint =
      cmd::kLastCommonId,  // All GLES2 commands start after this.
#define GLES2_CMD_OP(name) k##name,
  GLES2_COMMAND_LIST(GLES2_CMD_OP)
#undef GLES2_CMD_OP
      kNumCommands,
  kFirstGLES2Command = kOneBeforeStartPoint + 1
};

#endif  // GPU_COMMAND_BUFFER_COMMON_GLES2_CMD_IDS_AUTOGEN_H_
