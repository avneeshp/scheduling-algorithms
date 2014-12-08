; ModuleID = 'test2.c'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@out1 = common global i16 0, align 2
@out2 = common global i16 0, align 2
@out3 = common global i16 0, align 2

define void @test2(i16 signext %x1, i16 signext %y1, i16 signext %x2, i16 signext %y2,i16 signext %x3, i16 signext %y3, i16 signext %x4, i16 signext %y4, i16 signext %x5, i16 signext %y5, i16 signext %w, i16 signext %z) nounwind uwtable {
entry:
  %mul1 = mul i16 %x1, %y1
  %mul2 = mul i16 %x2, %y2
  %mul3 = mul i16 %mul1, %mul2
  %sub4 = sub i16 %mul3, 10
  %mul6 = mul i16 %x3, %y3
  %mul7 = mul i16 %mul6, 2
  %mul8 = mul i16 %x4, %y4
  %add9 = add i16 %mul8, 3
  %add10 = add i16 %x5, %y5
  %add11 = add i16 %add10, 5
  %sub5 = sub i16 %sub4 , %mul7
  store i16 %sub5, i16* @out1, align 2, !tbaa !0
  store i16 %add9, i16* @out2, align 2, !tbaa !0
  store i16 %add11, i16* @out3, align 2, !tbaa !0
  ret void
}

!0 = metadata !{metadata !"short", metadata !1}
!1 = metadata !{metadata !"omnipotent char", metadata !2}
!2 = metadata !{metadata !"Simple C/C++ TBAA"}
