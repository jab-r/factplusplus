cmake_minimum_required(VERSION 2.8.11)

set(SOURCE_FILES
	AD.cpp
	BSPO_Queries.h
	comparser.h
	comscanner.h
	FaCT.cpp
	lispgrammar.h
	LUBM2_Queries.h
	NCIT_Queries.h
	parser.cpp
	parser.h
	PO_Queries.h
	QA.cpp
	scanner.cpp
	scanner.h
)

# Executable
add_executable(FaCT++ ${SOURCE_FILES})
target_link_libraries(FaCT++ LINK_PUBLIC Kernel)
