; ModuleID = 'test4.ssa.ll'
source_filename = "test4.cpp"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

%struct.Foo = type { i32, i32, i32 }

@.str = private unnamed_addr constant [10 x i8] c"sum = %d\0A\00", align 1

; Function Attrs: noinline nounwind ssp uwtable
define void @_Z10modify_objP3Foo(%struct.Foo* %foo) #0 {
entry:
  %tobool = icmp ne %struct.Foo* %foo, null
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %x = getelementptr inbounds %struct.Foo, %struct.Foo* %foo, i32 0, i32 0
  %0 = bitcast i32* %x to i8*
  call void @nullcheck(i8* %0)
  store i32 1, i32* %x, align 4
  %y = getelementptr inbounds %struct.Foo, %struct.Foo* %foo, i32 0, i32 1
  %1 = bitcast i32* %y to i8*
  call void @nullcheck(i8* %1)
  store i32 2, i32* %y, align 4
  %z = getelementptr inbounds %struct.Foo, %struct.Foo* %foo, i32 0, i32 2
  %2 = bitcast i32* %z to i8*
  call void @nullcheck(i8* %2)
  store i32 3, i32* %z, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

; Function Attrs: noinline norecurse ssp uwtable
define i32 @main(i32 %argc, i8** %argv) #1 {
entry:
  %tmp = alloca %struct.Foo, align 4
  %cmp = icmp eq i32 %argc, 1
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %0 = bitcast %struct.Foo* %tmp to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %0, i8 0, i64 12, i1 false)
  %call = call i8* @_Znwm(i64 12) #5
  %1 = bitcast i8* %call to %struct.Foo*
  %2 = bitcast %struct.Foo* %1 to i8*
  call void @llvm.memset.p0i8.i64(i8* align 8 %2, i8 0, i64 12, i1 false)
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %foo.0 = phi %struct.Foo* [ %1, %if.then ], [ null, %entry ]
  call void @_Z10modify_objP3Foo(%struct.Foo* %foo.0)
  %x = getelementptr inbounds %struct.Foo, %struct.Foo* %foo.0, i32 0, i32 0
  %3 = load i32, i32* %x, align 4
  %y = getelementptr inbounds %struct.Foo, %struct.Foo* %foo.0, i32 0, i32 1
  %4 = load i32, i32* %y, align 4
  %add = add nsw i32 %3, %4
  %z = getelementptr inbounds %struct.Foo, %struct.Foo* %foo.0, i32 0, i32 2
  %5 = load i32, i32* %z, align 4
  %add1 = add nsw i32 %add, %5
  %call2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str, i64 0, i64 0), i32 %add1)
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #2

; Function Attrs: nobuiltin
declare noalias i8* @_Znwm(i64) #3

declare i32 @printf(i8*, ...) #4

define void @nullcheck(i8*) {
  %2 = icmp eq i8* %0, null
  br i1 %2, label %3, label %4

3:                                                ; preds = %1
  call void @exit(i32 1)
  unreachable

4:                                                ; preds = %1
  ret void
}

declare void @exit(i32)

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline norecurse ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { nobuiltin "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { builtin }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
