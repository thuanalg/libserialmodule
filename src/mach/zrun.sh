mv trigger_serial.txt trigger_serial.txt_
#You need "export" correctly. it depends on your setting, system.
export DYLD_LIBRARY_PATH=./:$DYLD_LIBRARY_PATH:/usr/local/thuannt
#export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH
./test_main --is_baudrate=115200 --is_cfg=simplelog.cfg --is_port=/dev/tty.Plser,/dev/tty.Plser1
#./test_main --is_baudrate=115200 --is_cfg=simplelog.cfg --is_port=/dev/ttyUSB0,/dev/ttyUSB1

