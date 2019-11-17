; ModuleID = 'test1-nullp.alias.ll'
source_filename = "test1-nullp.cpp"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

@nullp = common global i8* null, align 8
@0 = private unnamed_addr constant [34 x i8] c"Found a null pointer. Exiting...\0A\00", align 1

; Function Attrs: noinline nounwind ssp uwtable
define void @_Z6escapePv(i8* %p) #0 {
entry:
  call void asm sideeffect "", "imr,~{memory},~{dirflag},~{fpsr},~{flags}"(i8* %p) #2, !srcloc !3
  ret void
}

; Function Attrs: noinline nounwind ssp uwtable
define void @_Z10deref_nulli(i32 %argc) #0 {
entry:
  call void @_Z6escapePv(i8* bitcast (i8** @nullp to i8*))
  call void @nullcheck(i8* bitcast (i8** @nullp to i8*))
  store i32 100, i32* bitcast (i8** @nullp to i32*), align 4
  ret void
}

; Function Attrs: noinline norecurse nounwind ssp uwtable
define i32 @main(i32 %argc, i8** %argv) #1 {
entry:
  call void @_Z6escapePv(i8* bitcast (i8** @nullp to i8*))
  call void @_Z10deref_nulli(i32 %argc)
  ret i32 0
}

define void @nullcheck(i8*) {
  %2 = icmp eq i8* %0, null
  %3 = call i32 (...) @printf(i8* getelementptr inbounds ([34 x i8], [34 x i8]* @0, i32 0, i32 0))
  br i1 %2, label %4, label %5

4:                                                ; preds = %1
  call void @exit(i32 1)
  unreachable

5:                                                ; preds = %1
  ret void
}

declare i32 @printf(...)

declare void @exit(i32)

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline norecurse nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!3 = !{i32 3571987}
