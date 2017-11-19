## makefile (2017.11.19)
1. `某个makefile`
```
galen@66:/work/hardware/hello$ cat Makefile 
hello : hello.o a.o
	gcc -o $@ $^

%.o : %.c
	gcc -o 	$@ -c $<

clean :
	rm *.o hello

``` 
2. `$^`
```
所有的依赖目标的集合。以空格分隔。如果在依赖目标中有多个重复的,
那个这个变量会去除重复的依赖目标,只保留一份。
```
3. `$@ `
```
表示规则中的目标文件集。在模式规则中,如果有多个目标,那么,"$@"就是
匹配于目标中模式定义的集合。
```