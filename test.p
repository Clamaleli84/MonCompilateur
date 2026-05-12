VAR flag : BOOLEAN; flag2 : BOOLEAN; i : INTEGER; x : DOUBLE; y : DOUBLE; c : CHAR.
BEGIN
  x := 3.14;
  y := 2.0;
  x := x + y;
  DISPLAY x;
  x := 3.14;
  x := x - y;
  DISPLAY x;
  x := 3.14;
  x := x * y;
  DISPLAY x;
  x := 3.14;
  x := x / y;
  DISPLAY x;
  flag := 3.14 > 2.0;
  DISPLAY flag;
  flag := 3.14 < 2.0;
  DISPLAY flag;
  flag := 2.0 == 2.0;
  DISPLAY flag;
  flag := 2.0 != 3.14;
  DISPLAY flag;
  c := '\n';
  DISPLAY c;
  c := '\t';
  DISPLAY c;
  flag := 1 == 1;
  flag2 := 2 == 2;
  flag := flag || flag2;
  DISPLAY flag;
  flag := 1 == 1;
  flag2 := 0 == 1;
  flag := flag && flag2;
  DISPLAY flag;
  flag := 1 == 1;
  i := 1;
  WHILE flag DO
    BEGIN
      DISPLAY i;
      i := i + 1;
      flag := i <= 3
    END;
  i := 1;
  WHILE i <= 3 DO
    BEGIN
      IF i == 1 THEN
        BEGIN
          IF i == 1 THEN
            DISPLAY i
          ELSE
            DISPLAY i
        END
      ELSE
        DISPLAY i;
      i := i + 1
    END
END.