; ModuleID = 'pr.c'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@out1 = common global i16 0, align 2
@out2 = common global i16 0, align 2

define void @test1(i16 signext %x, i16 signext %y, i16 signext %w, i16 signext %z) nounwind uwtable {
entry:
  %sub1 = sub i16 %x, %y
  store i16 %sub1, i16* @out1, align 2, !tbaa !0
  %add2 = add i16 %x, %w
  %add3 = add i16 %add2, 10
  store i16 %add3, i16* @out2, align 2, !tbaa !0
  ret void
}

!0 = metadata !{metadata !"short", metadata !1}
!1 = metadata !{metadata !"omnipotent char", metadata !2}
!2 = metadata !{metadata !"Simple C/C++ TBAA"}
