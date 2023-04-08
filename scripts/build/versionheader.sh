#!/bin/bash
#
# Copyright (C) 2016 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#
#
# \brief	generate version header
#
# \param	target header



# arguments
header=$1

# get recent git tag
tag=$(git describe --abbrev=0 --tags 2>/dev/null || echo "-")

# get current git hash
hash=$(git rev-parse HEAD 2>/dev/null || echo "-")

# create target directory
mkdir -p $(dirname ${header})

# generate temporary header
cat > ${header}.tmp << EOL
#ifndef VERSION_H
#define VERSION_H


#define VERSION	"\tgit-tag: ${tag}\n\tgit-hash: ${hash}"


#endif // VERSION_H
EOL

# compare current and temporary header
if  ! diff ${header} ${header}.tmp 1>/dev/null 2>&1 ;then
	# update header
	cp ${header}.tmp ${header}
	echo "generating version header \"${header}\""
fi

# delete temporary header
rm ${header}.tmp

exit 0
