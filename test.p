VAR
  i, j, n, resultat : INTEGER;
  x, y : DOUBLE;
  c : CHAR;
  flag, ok : BOOLEAN;
  msg : STRING.

BEGIN


  i := 10;
  j := 3;
  DISPLAY i;
  DISPLAY j;


  resultat := i + j;   DISPLAY resultat;
  resultat := i - j;   DISPLAY resultat;
  resultat := i * j;   DISPLAY resultat;
  resultat := i / j;   DISPLAY resultat;
  resultat := i % j;   DISPLAY resultat;


  x := 3.14;
  y := 2.0;
  DISPLAY x;
  DISPLAY y;
  x := 3.14 + 2.0;   DISPLAY x;
  x := 10.0 - 3.5;   DISPLAY x;
  x := 2.5 * 4.0;    DISPLAY x;
  x := 10.0 / 4.0;   DISPLAY x;


  c := 'A';   DISPLAY c;
  c := 'z';   DISPLAY c;


  DISPLAY "Hello, World!";
  DISPLAY "Test du compilateur CERI";

  flag := i == 10;   DISPLAY flag;
  flag := i != j;    DISPLAY flag;
  flag := j < i;     DISPLAY flag;
  flag := i > j;     DISPLAY flag;
  flag := i <= 10;   DISPLAY flag;
  flag := j >= 3;    DISPLAY flag;

  ok := (i == 10) && (j == 3);   DISPLAY ok;
  ok := (i == 99) || (j == 3);   DISPLAY ok;
  ok := (i == 99) && (j == 3);   DISPLAY ok;


  IF i > 5 THEN
    DISPLAY "i est grand"
  ELSE
    DISPLAY "i est petit";

  IF j == 3 THEN
    DISPLAY "j vaut 3";

  i := 2;
  IF i > 5 THEN
    DISPLAY "i est grand"
  ELSE
    DISPLAY "i est petit";
  i := 10;


  n := 1;
  WHILE n <= 5 DO
    BEGIN
      DISPLAY n;
      n := n + 1
    END;


  FOR i := 1 TO 5 DO
    DISPLAY i;

  FOR i := 5 DOWNTO 1 DO
    DISPLAY i;

  n := 1;
  REPEAT
    DISPLAY n;
    n := n + 1
  UNTIL n > 5;

  n := 1;
  DO
    BEGIN
      DISPLAY n;
      n := n + 1
    END
  WHILE n <= 5;

  FOR j := 1 TO 3 DO
    CASE j OF
      1 : DISPLAY "un";
      2 : DISPLAY "deux";
      3 : DISPLAY "trois"
    END;

  c := 'a';
  CASE c OF
    'a' : DISPLAY "voyelle a";
    'e' : DISPLAY "voyelle e";
    'b' : DISPLAY "consonne"
  END;

  c := 'e';
  CASE c OF
    'a' : DISPLAY "voyelle a";
    'e' : DISPLAY "voyelle e";
    'b' : DISPLAY "consonne"
  END;

  c := 'b';
  CASE c OF
    'a' : DISPLAY "voyelle a";
    'e' : DISPLAY "voyelle e";
    'b' : DISPLAY "consonne"
  END;

  BEGIN
    i := 42;
    j := 58;
    resultat := i + j;
    DISPLAY resultat
  END;


  resultat := (i + j) * 2 - 10 / 5 % 3;
  DISPLAY resultat

END.
