VAR i : INTEGER.
BEGIN
  i := 5;
  CASE i OF
    1 : DISPLAY i;
    2 : DISPLAY i;
    3 : DISPLAY i
  END;
  DISPLAY i;
  i := 1;
  CASE i OF
    1, 2, 3 : DISPLAY i
  END;
  i := 2;
  CASE i OF
    1, 2, 3 : DISPLAY i
  END
END.