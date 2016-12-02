В этой задаче требуется найти оптимальный период для бесконечной десятичной дроби. Рассмотрим бесконечную десятичную дробь x0.x1x2x3..., которая является записью неко- торого вещественного числа x от 0 до 1 включительно: x = x0+x1*10-1+x2*10-2+x3*10-3 + ... . Здесь xi — это десятичные цифры от 0 до 9. В этой задаче нет никаких ограничений на дробь, кроме приведённых выше. В частности, это означает, что, например, 0.999999... и 1.000000... — корректные бесконечные десятичные дроби, являющиеся записью одного и того же вещественного числа 1.
Периодическая десятичная дробь — это способ записи бесконечной десятичной дроби в виде , где r > 0 и s > r. Эту запись можно раскрыть в бесконеч- ную десятичную дробь , то есть бесконеч- ную дробь, начинающуюся с y0.y1y2y3… yr и затем повторяющую последовательность цифр yr+1yr+2… ys в бесконечном цикле.
Будем говорить, что r — это длина предпериода, а s-r — это длина периода. Не всякую бесконечную десятичную дробь можно записать как периоди- ческую. На самом деле такое представление существует тогда и только тогда, когда веще- ственное число является рациональным.
Нам заданы несколько первых цифр бесконечной десятичной дроби, оставшиеся циф- ры просто отброшены (никакого округления не происходит). Теперь мы хотим записать какую-нибудь периодическую десятичную дробь, раскрыв которую, мы получим дробь, начи- нающуюся с заданной конечной части. Среди таких бесконечных десятичных дробей найдите ту, у которой сумма длин предпериода и периода минимально возможная.