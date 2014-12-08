; ModuleID = 'honda.c'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@S_1 = common global i16 0, align 2
@S_2 = common global i16 0, align 2
@S_3 = common global i16 0, align 2
@S_4 = common global i16 0, align 2
@S_5 = common global i16 0, align 2
@S_6 = common global i16 0, align 2
@S_7 = common global i16 0, align 2
@Y_0_1 = common global i16 0, align 2
@Y_1_1 = common global i16 0, align 2

define void @honda(i16 signext %X_0_1, i16 signext %X_1_1, i16 signext %S_1_1, i16 signext %S_2_1, i16 signext %S_3_1, i16 signext %S_4_1, i16 signext %S_5_1, i16 signext %S_6_1, i16 signext %S_7_1) nounwind uwtable {
entry:
  %mul = mul i16 %S_1_1, 8263
  %mul4 = mul i16 %S_2_1, 1298
  %mul8 = mul i16 %S_3_1, 8034
  %mul13 = mul i16 %S_4_1, 8411
  %mul18 = mul i16 %S_5_1, 3718
  %mul23 = mul i16 %X_0_1, 2007
  %tmp0 = mul i16 %X_1_1, -1491
  %add = add i16 %tmp0, %mul23
  %add11 = add i16 %add, %mul
  %add16 = add i16 %add11, %mul4
  %add21 = add i16 %add16, %mul8
  %add26 = add i16 %add21, %mul13
  %sub = add i16 %add26, %mul18
  store i16 %sub, i16* @S_1, align 2, !tbaa !0
  %mul33 = mul i16 %S_1_1, 4418
  %mul37 = mul i16 %S_2_1, 3784
  %mul42 = mul i16 %S_3_1, 1627
  %mul47 = mul i16 %S_4_1, 5249
  %mul52 = mul i16 %S_5_1, 2596
  %tmp1 = mul i16 %X_0_1, -5601
  %mul62 = mul i16 %X_1_1, 2981
  %add40 = add i16 %mul62, %tmp1
  %add45 = add i16 %add40, %mul33
  %add50 = add i16 %add45, %mul37
  %add55 = add i16 %add50, %mul42
  %sub60 = add i16 %add55, %mul47
  %add65 = add i16 %sub60, %mul52
  store i16 %add65, i16* @S_2, align 2, !tbaa !0
  %mul68 = mul i16 %S_1_1, 6321
  %mul72 = mul i16 %S_2_1, 1034
  %mul77 = mul i16 %S_3_1, 1153
  %tmp2 = mul i16 %S_4_1, -9355
  %tmp3 = mul i16 %S_5_1, -4732
  %tmp4 = mul i16 %X_0_1, -1130
  %mul97 = mul i16 %X_1_1, 1019
  %add75 = add i16 %mul97, %tmp4
  %add80 = add i16 %add75, %mul68
  %sub85 = add i16 %add80, %mul72
  %sub90 = add i16 %sub85, %mul77
  %sub95 = add i16 %sub90, %tmp2
  %add100 = add i16 %sub95, %tmp3
  store i16 %add100, i16* @S_3, align 2, !tbaa !0
  %mul103 = mul i16 %S_1_1, 4548
  %mul107 = mul i16 %S_2_1, 2553
  %mul112 = mul i16 %S_3_1, 5567
  %mul117 = mul i16 %S_4_1, 4094
  %mul122 = mul i16 %S_5_1, 1360
  %mul127 = mul i16 %X_0_1, 2145
  %tmp5 = mul i16 %X_1_1, -4924
  %add110 = add i16 %tmp5, %mul127
  %add115 = add i16 %add110, %mul103
  %add120 = add i16 %add115, %mul107
  %add125 = add i16 %add120, %mul112
  %add130 = add i16 %add125, %mul117
  %sub135 = add i16 %add130, %mul122
  store i16 %sub135, i16* @S_4, align 2, !tbaa !0
  %mul138 = mul i16 %S_1_1, 5087
  %mul142 = mul i16 %S_2_1, 1056
  %tmp6 = mul i16 %S_3_1, -3947
  %mul152 = mul i16 %S_4_1, 4757
  %mul157 = mul i16 %S_5_1, 9659
  %tmp7 = mul i16 %X_0_1, -1702
  %mul167 = mul i16 %X_1_1, 5519
  %add145 = add i16 %mul167, %tmp7
  %sub150 = add i16 %add145, %mul138
  %add155 = add i16 %sub150, %mul142
  %add160 = add i16 %add155, %tmp6
  %sub165 = add i16 %add160, %mul152
  %add170 = add i16 %sub165, %mul157
  store i16 %add170, i16* @S_5, align 2, !tbaa !0
  %mul173 = mul i16 %S_1_1, 9670
  %mul177 = mul i16 %S_2_1, 4141
  %mul182 = mul i16 %S_3_1, 1503
  %mul187 = mul i16 %S_4_1, 7286
  %mul192 = mul i16 %S_5_1, 6262
  %tmp8 = mul i16 %X_0_1, -2114
  %mul202 = mul i16 %X_1_1, 2007
  %add180 = add i16 %mul202, %tmp8
  %add185 = add i16 %add180, %mul173
  %add190 = add i16 %add185, %mul177
  %add195 = add i16 %add190, %mul182
  %sub200 = add i16 %add195, %mul187
  %add205 = add i16 %sub200, %mul192
  store i16 %add205, i16* @S_6, align 2, !tbaa !0
  %mul208 = mul i16 %S_1_1, 1317
  %mul212 = mul i16 %S_2_1, 4936
  %mul217 = mul i16 %S_3_1, 2123
  %mul222 = mul i16 %S_4_1, 1148
  %mul227 = mul i16 %S_5_1, 4344
  %mul232 = mul i16 %X_0_1, 1194
  %tmp9 = mul i16 %X_1_1, -2114
  %add215 = add i16 %tmp9, %mul232
  %add220 = add i16 %add215, %mul208
  %add225 = add i16 %add220, %mul212
  %add230 = add i16 %add225, %mul217
  %add235 = add i16 %add230, %mul222
  %sub240 = add i16 %add235, %mul227
  store i16 %sub240, i16* @S_7, align 2, !tbaa !0
  %tmp10 = mul i16 %X_0_1, -1491
  %sub247 = add i16 %tmp10, %S_6_1
  store i16 %sub247, i16* @Y_0_1, align 2, !tbaa !0
  %sub259 = add i16 %add, %S_7_1
  store i16 %sub259, i16* @Y_1_1, align 2, !tbaa !0
  ret void
}

!0 = metadata !{metadata !"short", metadata !1}
!1 = metadata !{metadata !"omnipotent char", metadata !2}
!2 = metadata !{metadata !"Simple C/C++ TBAA"}
