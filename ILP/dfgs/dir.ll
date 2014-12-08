; ModuleID = 'dir.c'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@y10 = common global i16 0, align 2
@y11 = common global i16 0, align 2
@y12 = common global i16 0, align 2
@y13 = common global i16 0, align 2
@y14 = common global i16 0, align 2
@y15 = common global i16 0, align 2
@y16 = common global i16 0, align 2
@y17 = common global i16 0, align 2

define void @dir(i16 signext %x0, i16 signext %x1, i16 signext %x2, i16 signext %x3, i16 signext %x4, i16 signext %x5, i16 signext %x6, i16 signext %x7) nounwind uwtable {
entry:
  %mul320 = add i16 %x1, %x0
  %mul8321 = add i16 %mul320, %x2
  %add11322 = add i16 %mul8321, %x3
  %add16323 = add i16 %add11322, %x4
  %add21324 = add i16 %add16323, %x5
  %add26325 = add i16 %add21324, %x6
  %add31326 = add i16 %add26325, %x7
  %add36 = mul i16 %add31326, 3535
  store i16 %add36, i16* @y10, align 2, !tbaa !0
  %tmp = sub i16 %x3, %x4
  %tmp355 = mul i16 %tmp, 975
  %tmp357 = sub i16 %x2, %x5
  %tmp358 = mul i16 %tmp357, 2777
  %tmp360 = sub i16 %x1, %x6
  %tmp361 = mul i16 %tmp360, 4157
  %tmp363 = sub i16 %x0, %x7
  %tmp364 = mul i16 %tmp363, 4903
  %add66 = add i16 %tmp358, %tmp355
  %add71 = add i16 %add66, %tmp361
  %add76 = add i16 %add71, %tmp364
  store i16 %add76, i16* @y11, align 2, !tbaa !0
  %tmp367 = sub i16 %x1, %x2
  %tmp368 = sub i16 %tmp367, %x5
  %tmp369 = add i16 %tmp368, %x6
  %tmp370 = mul i16 %tmp369, 1913
  %tmp373 = sub i16 %x0, %x3
  %tmp374 = sub i16 %tmp373, %x4
  %tmp375 = add i16 %tmp374, %x7
  %tmp376 = mul i16 %tmp375, 4619
  %add116 = add i16 %tmp376, %tmp370
  store i16 %add116, i16* @y12, align 2, !tbaa !0
  %tmp378 = sub i16 %x4, %x3
  %tmp379 = mul i16 %tmp378, 2777
  %tmp381 = sub i16 %x5, %x2
  %tmp382 = mul i16 %tmp381, 4903
  %tmp384 = sub i16 %x6, %x1
  %tmp385 = mul i16 %tmp384, 975
  %tmp388 = mul i16 %tmp363, 4157
  %add146 = add i16 %tmp382, %tmp379
  %add151 = add i16 %add146, %tmp385
  %add156 = add i16 %add151, %tmp388
  store i16 %add156, i16* @y13, align 2, !tbaa !0
  %tmp393 = sub i16 %x0, %x1
  %tmp394 = sub i16 %tmp393, %x2
  %tmp395 = add i16 %tmp394, %x3
  %tmp396 = add i16 %tmp395, %x4
  %tmp397 = sub i16 %tmp396, %x5
  %tmp398 = sub i16 %tmp397, %x6
  %tmp399 = add i16 %tmp398, %x7
  %tmp400 = mul i16 %tmp399, 3535
  store i16 %tmp400, i16* @y14, align 2, !tbaa !0
  %tmp403 = mul i16 %tmp, 4157
  %tmp406 = mul i16 %tmp357, 975
  %tmp409 = mul i16 %tmp384, 4903
  %tmp412 = mul i16 %tmp363, 2777
  %add226 = add i16 %tmp406, %tmp403
  %add231 = add i16 %add226, %tmp409
  %add236 = add i16 %add231, %tmp412
  store i16 %add236, i16* @y15, align 2, !tbaa !0
  %tmp415 = sub i16 %x2, %x1
  %tmp416 = add i16 %tmp415, %x5
  %tmp417 = sub i16 %tmp416, %x6
  %tmp418 = mul i16 %tmp417, 4619
  %tmp424 = mul i16 %tmp375, 1913
  %add276 = add i16 %tmp424, %tmp418
  store i16 %add276, i16* @y16, align 2, !tbaa !0
  %tmp427 = mul i16 %tmp378, 4903
  %tmp430 = mul i16 %tmp357, 4157
  %tmp433 = mul i16 %tmp384, 2777
  %tmp436 = mul i16 %tmp363, 975
  %add306 = add i16 %tmp430, %tmp427
  %add311 = add i16 %add306, %tmp433
  %add316 = add i16 %add311, %tmp436
  store i16 %add316, i16* @y17, align 2, !tbaa !0
  ret void
}

!0 = metadata !{metadata !"short", metadata !1}
!1 = metadata !{metadata !"omnipotent char", metadata !2}
!2 = metadata !{metadata !"Simple C/C++ TBAA"}
