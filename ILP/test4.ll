; ModuleID = 'test4.c'
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

define void @test4(i16 signext %x10, i16 signext %x11, i16 signext %x12, i16 signext %x13, i16 signext %x14, i16 signext %x15, i16 signext %x16, i16 signext %x17) nounwind uwtable {
entry:
  %add = add i16 %x17, %x10
  %add5 = add i16 %x16, %x11
  %add9 = add i16 %x14, %x13
  %add13 = add i16 %x15, %x12
  %add43 = add i16 %add, %add9
  %sub = sub i16 %x10, %x17
  %sub20 = sub i16 %x11, %x16
  %sub24 = sub i16 %x13, %x14
  %sub28 = sub i16 %x12, %x15
  %mul = mul i16 %sub, 5098
  %mul33 = mul i16 %sub20, 6013
  %mul36 = mul i16 %sub24, 25629
  %mul39 = mul i16 %sub28, 8999
  %add47 = add i16 %add5, %add13
  %sub51 = sub i16 %add, %add9
  %sub55 = sub i16 %add5, %add13
  %add59 = add i16 %mul, %mul36
  ret void
}

!0 = metadata !{metadata !"short", metadata !1}
!1 = metadata !{metadata !"omnipotent char", metadata !2}
!2 = metadata !{metadata !"Simple C/C++ TBAA"}
