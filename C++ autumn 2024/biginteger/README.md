# B. BigInteger+Rational

|   |   |
|---|---|
| Ограничение времени |	10 секунд |
| Ограничение памяти |	64Mb |
| Ввод	стандартный | ввод или input.txt |
| Вывод	стандартный | вывод или output.txt |

Напишите класс `BigInteger` для работы с длинными целыми числами. Должны поддерживаться операции:

- сложение, вычитание, умножение, деление, остаток по модулю, работающие так же, как и для `int`; составное присваивание с этими операциями. Деление должно работать не дольше, чем за $O(n^2)$.
- унарный минус, префиксный и постфиксный инкремент и декремент. Префиксный инкремент и декремент должны работать за $O(1)$ в среднем.
- операторы сравнения $==$ $!=$ $<$ $>$ $<=$ $>=$. Опционально можете реализовать оператор $<=>$ и выразить сравнения через него, как это стало возможно в C++20.
- вывод в поток и ввод из потока
- метод `toString()`, возвращающий строковое представление числа
- конструирование из `int` (в том числе неявное преобразование, когда это надо)
- неявное преобразование в `bool`, когда это надо (должно работать в условных выражениях)
- литеральный суффикс `bi` для написания литералов типа `BigInteger`.

Используя класс `BigInteger`, напишите класс `Rational` для работы с рациональными числами сколь угодно высокой точности. Числа `Rational` должны представляться в виде несократимых обыкновенных дробей, где числитель и знаменатель – сколь угодно длинные целые числа. Должны поддерживаться операции:
- конструктор из `BigInteger`, из `int`
- сложение, вычитание, умножение, деление, составное присваивание с этими операциями
- унарный минус
- операторы сравнения $==$ $!=$ $<$ $>$ $<=$ $>=$. Опционально можете реализовать оператор $<=>$ и выразить сравнения через него, как это стало возможно в C++20.
- метод `toString()`, возвращающий строковое представление числа (вида [минус]числитель/знаменатель), где числитель и знаменатель - взаимно простые числа; если число на самом деле целое, то знаменатель выводить не надо
- метод `asDecimal(size_t precision=0)`, возвращающий строковое представление числа в виде десятичной дроби с `precision` знаками после запятой
- оператор приведения к `double`

В вашем файле должна отсутствовать функция `main()`, а сам файл должен называться `biginteger.h`. В качестве компилятора необходимо указывать GCC C++17 Make. Ваш код будет вставлен посредством в программу, содержащую тесты; вследствие этого код необходимо отправлять в файле со строго соответствующим именем!