#!/bin/sh

if [ "$1" = "" ] ; then
  echo "usage $0 <crypto> <zip> <slice> <Slice> <tape mark[y|n]> <sequential read[y|n]> <min-digit> <sparse-size> <keepcompressed[y|n]> <recheck-hole[y|n]> <hash>"
  exit 1
fi

crypto="$1"
zip="$2"
slice="$3"
Slice="$4"
tape_mark="$5"
seq_read="$6"
digit="$7"
sparse="$8"
keep_compr="$9"
re_hole="${10}"
hash="${11}"

#echo "crypto = $crypto"
#echo "zip = $zip"
#echo "slice = $slice"
#echo "Slice = $Slice"
#echo "tape_mark = $tape_mark"
#echo "seq_read = $seq_read"
#echo "digit = $digit"
#echo "sparse = $sparse"
#echo "keep_compr = $keep_compr"
#echo "re_hole = $re_hole"
#echo "hash = [$hash]"

ALL_TESTS="A1 B1 B2 B3 B4 C1 C2 C3 C4 D1 E1 E2 E3 F1 F2 F3"

export OPT=tmp.file

if [ "$crypto" != "none" ]; then
  crypto_K="-K $crypto:toto"
  crypto_J="-J $crypto:toto"
  crypto_A="'-$' $crypto:toto"
else
  crypto_K=""
  crypto_J=""
  crypto_A=""
fi

if [ "$zip" != "none" ]; then
  zip=-z$zip
else
  zip=""
fi

if [ "$slice" != "none" ]; then
  slicing="-s $slice"
  if [ "$Slice" != "none" ]; then
    slicing="$slicing -S $Slice"
  fi
else
  slicing=""
fi


if [ "$tape_mark" = "y" ]; then
  tape=""
else
  tape="-at"
fi

if [ "$seq_read" = "y" ]; then
  sequential="--sequential-read"
  ALL_TESTS=`echo $ALL_TESTS | sed -r -e 's/(F1|F2|F3)//g'`
  if [ "$tape_mark" != "y" ] ; then
    ALL_TESTS="none"
  fi
else
  sequential=""
fi

if [ "$digit" != "none" ]; then
  min_digits="--min-digit $digit,$digit,$digit"
else
  min_digits=""
fi

sparse="-1 $sparse"

if [ "$keep_compr" = "y" ]; then
  keepcompressed="-ak"
else
  keepcompressed=""
fi

if [ "$re_hole" = "y" ]; then
  recheck_hole="-ah"
else
  recheck_hole=""
fi

if [ "$hash" != "none" ]; then
  hash="--hash $hash"
else
  hash=""
fi

cat > $OPT <<EOF
all:
 $sequential
 $min_digits
 $crypto_K

reference:
 $crypto_J

auxiliary:
 $crypto_A

isolate:
 $zip
 $slicing
 $tape
 $hash


merge:
 $zip
 $slicing
 $tape
 $hash
 $keepcompressed
 $recheck_hole

create:
 $zip
 $slicing
 $tape
 $hash
 $sparse

listing:

testing:

diffing:

EOF
echo "----------------------------------------------------------------"
echo "----------------------------------------------------------------"
echo "TESTS PARAMETERS NATURE: crypto zip slice Slice tape-mark seq-read digit sparse keepcompr re-hole hash"
echo "TESTS PARAMETERS VALUE: $*"
echo "TEST SET: $ALL_TESTS"
echo "----------------------------------------------------------------"
exec ./routine.sh $ALL_TESTS
