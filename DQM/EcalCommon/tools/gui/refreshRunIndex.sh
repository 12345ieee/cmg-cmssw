#!/bin/sh

if [ $# -lt 1 ]; then
  echo 'Usage: updateRunIndex.sh NUMBER-OF-FILES'
  exit
fi

N=$1

ulimit -c 0

cd ${HOME}/DQM/dqm-GUI

LD_LIBRARY_PATH=
source rpms/cmsset_default.sh
source rpms/slc4_ia32_gcc345/cms/dqmgui/5.1.6/etc/profile.d/env.sh

if [ -e /tmp/updateRunIndex.lock ]; then
  echo "Lock file is present, exit"
  exit 1
fi

touch /tmp/updateRunIndex.lock

echo "Index refresh: begin"

FILES=`find /data/ecalod-disk01/dqm-data/root/ -name 'DQM_V*.root' -mtime -1 | xargs -r ls -tr | tail -$N`

for F in $FILES; do
  echo "Remove: "$F
  visDQMIndex remove --dataset /Global/Online/ALL /data/ecalod-disk01/dqm-GUI/idx $F
 echo "Add: "$F
  visDQMIndex add --dataset /Global/Online/ALL /data/ecalod-disk01/dqm-GUI/idx $F
done

echo "Index refresh: end"

rm /tmp/updateRunIndex.lock

exit 0

