# 语法说明

```
<程序> ::= <分程序>.
<分程序> ::= [<类型说明部分>][<常量说明部分>][<变量说明部分>][<模块说明部分>]<语句>
<类型说明部分> ::= <重定义类型> | <枚举类型> | <结构体类型>
<重定义类型> ::= TYPE <标识符>=<标识符>;
<枚举类型> ::= TYPE <标识符>='{' <标识符> {, <标识符>} '}';
<结构体类型> ::= TYPE <标识符>=RECORD; <变量及类型>{; <变量及类型>}; END;
<常量说明部分> ::= CONST <常量定义> {, <常量定义>};
<常量定义> ::= <标识符>=<字面量>
<字面量> ::= <有符号整数>|<有符号小数>|<字符串常量>
<变量说明部分> ::= VAR <变量及类型>{, <变量及类型>};
<变量及类型> ::= <标识符>['['<区间说明>{, <区间说明>}']'] : <类型>
<区间说明> ::= <整数或常量> : <整数或常量>
<模块说明部分> ::= (<过程说明部分> | <函数说明部分>){; <模块说明部分>};
<过程说明部分> ::= <过程说明首部><分程序>
<函数说明部分> ::= <函数说明首部><分程序>
<过程说明首部> ::= PROCEDURE<标识符>(<参数列表>);
<函数说明首部> ::= FUNCTION<标识符>(<参数列表>):<类型>;
<参数列表> ::= (<变量及类型> {, <变量及类型>})|<空>
<语句> ::= <赋值语句>|<条件语句>|<循环语句>|<过程调用语句>|<读语句>|<写语句>|<复合语句>|<返回语句>|<空>
<赋值语句> ::= <左值表达式>:=<表达式>
<左值表达式> ::= <标识符>['['<区间表达式>'{,<区间表达式>}']']{. <左值表达式>}
<复合语句> ::= BEGIN<语句>{; <语句>}END
<条件表达式> ::= <或条件> {OR <或条件>}
<或条件> ::= <条件> {AND <条件>}
<条件> ::= <表达式><关系运算符><表达式>|ODD<表达式>
<表达式> ::= [+|-]<项>{<加法运算符><项>}
<项> ::= <因子>{<乘法运算符><因子>}
<因子> ::= <左值表达式>|<字面量>|'(' <表达式> ')'|<调用语句>
<加法运算符> ::= +|-
<乘法运算符> ::= *|/|MOD|%
<关系运算符> ::= =|#|<|<=|>|>=|!=
<条件语句> ::= <IF条件语句> | <CASE条件语句>
<IF条件语句> ::= IF<条件表达式>THEN<语句>{ELSEIF<条件表达式>THEN<语句>}[ELSE<语句>]
<CASE条件语句> ::= CASE <表达式> OF <CASE从句>{; <CASE从句>} [; <ELSE从句>]; END
<调用语句> ::= [CALL]<标识符>'(' [<表达式> {,<表达式>}]')'
<循环语句> ::= <当型循环语句>|<直到型循环语句>|<FOR循环语句>
<当型循环语句> ::= WHILE<条件表达式>DO<语句>
<直到型循环语句> ::= REPEAT <语句> UNTIL <条件表达式>
<FOR循环语句> ::= FOR <左值表达式> := <表达式> STEP <表达式> UNTIL <表达式> DO <语句>
<读语句> ::= READ'('<左值表达式>{, <左值表达式>}')'
<写语句> ::= WRITE'('<表达式>{, <表达式>}')'
<返回语句> ::= RETURN <表达式>
<字母> ::= a-zA-Z
<数字> ::= 0-9
<字符串常量> ::= "<非转义引号外任意字符>"
<注释> ::= <行注释> | <块注释>
<行注释> ::= // <任意字符> <换行符>
<块注释> ::= /* <任意字符> */
```
## 一些语法结构对应的中间代码

1. `if` 条件语句

   ```
   if <condition1> then
       <statement1>
   elseif <condition2> then
       <statement2>
   elseif <condition3> then
       <statement3>
   else
       <statement_n>
   ```

   翻译后的结构应该是：

   ```
   0x00 (<condition1>)
   0x01 (jz, 0x04, null, null)
   0x02 (<statement1>)
   0x03 (jmp, 0x0d, null, null)
   0x04 (<condition2>)
   0x05 (jz, 0x08, null, null)
   0x06 (<statement2>)
   0x07 (jmp, 0x0d, null, null)
   0x08 (<condition3>)
   0x09 (jz, 0x0c, null, null)
   0x0a (<statement3>)
   0x0b (jmp, 0x0d, null, null)
   0x0c (<statement_n>)
   0x0d (<外围语句>)
   ```

2. `case` 条件语句

   ```
   case <expression> of
   val1: 
       <statement1>;
   val2:
       <statement2>;
   else
       <statement_n>;
   end;
   ```

   翻译后的结构应该是：

   ```
   0x00 (<expression>)
   0x01 (cmp, val1, s[top], null) // 比较栈顶和数，并将比较结果压栈
   0x02 (jnz, 0x05, null, null)       // 将栈顶的数和 0 比较，并出栈
   0x03 (<statement1>)
   0x04 (jmp, 0x0a, null, null)
   0x05 (cmp, val2,s[top], null)
   0x06 (jnz, 0x09, null, null)
   0x07 (<statement2>)
   0x08 (jmp, 0x0a, null, null)
   0x09 (<statement_n>)
   0x0a (<外围语句>)
   ```

3. `while` 语句

   ```
   while <condition> do
       <statement>
   ```

   翻译后的结果应该是：

   ```
   0x00 (<condition>)
   0x01 (jz, 0x03, null, null)
   0x02 (<statement>)
   0x03 (jmp, 0x00, null, null)
   0x03 (<外围语句>)
   ```

4. `repeat` 语句

   ```
   repeat
       <statement>
   until <condition>;
   ```

   翻译后的结果应该是：

   ```
   0x00 (<statement>)
   0x01 (<condition>)
   0x02 (jz, 0x00, null, null)
   0x03 (<外围语句>)
   ```

5. `for` 语句

   ```
   for <var>:=<expression1> step<exression2> until <expression3> do
       <statement>
   ```

   翻译后的结果应该是：

   ```
   0x00 (<expression1>)
   0x01 (<expression2>) // 如果没有 step，就为 (push, 1, null, null)
   0x02 (<expression3>)
   0x03 (mov, s[top-2], null, <var>)
   0x04 (<statement>)
   0x05 (push, <var>, null, null)
   0x06 (push, s[top-2], null, null)
   0x07 (add, null, null, null)
   0x08 (cmp, s[top-1], s[top], null)
   0x09 (jle, 0x03, null, null)
   0x0a (<外围代码>)
   ```

6. `read` 和 `write` 语句

   ```
   read(<var>, <var>, <var>);
   ```

   翻译后结果为：

   ```
   (read, null, null, null)		// 读取一个整数并压栈
   (pop, <var>, null, null)
   ...
   ```

四元式：

push_const, val, null, null
push_var