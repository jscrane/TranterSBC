These are examples from [here](https://github.com/jefftranter/6502/tree/master/sbc/examples).

I used [this assembler](https://www.masswerk.at/6502/assembler.html), downloaded as binary and then
converted to S-Records:

```shell
srec_cat button.bin -binary -offset 0x1000 -o button.srec
```

They can then be uploaded over minicom (as ascii files) into JMON.
