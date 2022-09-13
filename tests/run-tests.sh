#!/bin/bash

function run_test {
	local name="$1"
	cd "$name"
	if "$RISCV/bin/riscv64-unknown-elf-gcc" "$name.c" -o "$name" &>/dev/null; then
		if "$RISCV/bin/tag-parser" "$name" "$name.tags" "$name.policy" &>/dev/null; then
			"$RISCV/bin/spike-tag" "--tag-files=policy.d2sc,tags.d2sc" "pk" "$name" >"$name-tags.out" 2>"$name-tags.time"
			"$RISCV/bin/spike" "pk" "$name" >"$name.out" 2>"$name.time"
			echo -e "\tSuccess"
		else
			echo "Failed to run tag-praser for case $name"
		fi
	else
		echo "Failed to run test case $name!"
	fi
	cd ".."
}

if [[ -z "$RISCV" ]]; then
	echo 'Please set the $RISCV variable!'
	echo 'It should be set to the RISCV installation path!'
	exit 1
fi

for ITEM in $(ls); do
	if [[ -d "$ITEM" ]]; then
		echo -n "Running test case $ITEM"
		run_test "$ITEM"
	fi
done
