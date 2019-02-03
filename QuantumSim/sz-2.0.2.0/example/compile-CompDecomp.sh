SZPATH=/usr/local
ZCPATH=/usr/local
SZFLAG="-I$SZPATH/include -I$ZCPATH/include $SZPATH/lib/libsz.a $SZPATH/lib/libzlib.a $ZCPATH/lib/libzc.a"
#echo rm testfloat_CompDecomp 
if [ -f testfloat_CompDecomp ]; then
	rm testfloat_CompDecomp
fi
echo gcc -lm -g -o testfloat_CompDecomp testfloat_CompDecomp.c ${SZFLAG}
gcc -lm -g -o testfloat_CompDecomp testfloat_CompDecomp.c $SZFLAG
