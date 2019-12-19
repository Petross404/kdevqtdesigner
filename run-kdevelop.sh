#!/bin/sh

PLUGIN_DIR=`pwd`/plugins

if test -d /usr/lib64 ; then
   LIB_DIR="lib64"
else
   LIB_DIR="lib"
fi

insert_first () {
   local NAME=$1
   local NEW=$2
   eval VALUE=\"\$$NAME\"

   if test -z "$VALUE" ; then
      export $NAME="$NEW"
   else
      export $NAME="$NEW:$VALUE"
   fi
}

insert_last () {
   local NAME=$1
   local NEW=$2
   eval VALUE=\"\$$NAME\"

   if test -z "$VALUE" ; then
      export $NAME="$NEW"
   else
      export $NAME="$VALUE:$NEW"
   fi
}

insert_last  XDG_CONFIG_DIRS /etc/xdg
insert_last  XDG_DATA_DIRS /usr/share

insert_first KDEDIRS $PLUGIN_DIR

insert_first PATH $PLUGIN_DIR/bin
insert_first LIBRARY_PATH $PLUGIN_DIR/$LIB_DIR
insert_first LD_LIBRARY_PATH $PLUGIN_DIR/$LIB_DIR

insert_first QT_PLUGIN_PATH $PLUGIN_DIR/$LIB_DIR/plugins

insert_first XDG_DATA_DIRS $PLUGIN_DIR/share

kdevelop $*

