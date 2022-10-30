#!/bin/bash

# These commands were used to time the test case
# Make sure that the $RISCV variable is set to the RISC-V installation path

function run-time {
	local name="$1"

	if [[ -z "$name" ]]; then
		echo "No test case name specified!"
		return 1
	fi

	if [[ ! -d "tags" ]]; then
		mkdir "tags"
	fi

	if [[ ! -d "no-tags" ]]; then
		mkdir "no-tags"
	fi

	# Compile the test case
	"$RISCV/bin/riscv64-unknown-elf-gcc" "$name.c" -o "$name"

	# Generate tag and policy files
	tag-parser "$name" "$name.tags" "$name.policy"

	# Run times
	if [[ -f time.out ]]; then
		rm time.out
	fi
	if [[ -f time-tags.out ]]; then
		rm time-tags.out
	fi
	if [[ -f time-no-tags.out ]]; then
		rm time-no-tags.out
	fi
	
	for ((i = 1; i <= 500; i++)); do
		spike pk "$name" &>/dev/null

		spike-tag pk "$name" &>/dev/null

		spike-tag --tag-files=policy.mtag,tags.mtag pk "$name" &>/dev/null
	done

	python time-calc.py time.out time-no-tags.out time-tags.out "$name:$directory"
}

run-time "floats"
