MACRO(ENUMINIT)

	IF (NOT EXISTS "${PROJECT_SOURCE_DIR}/code/engine/Core/Enums.hh")

		IF (WIN32)
			SET (ENUMINIT_SCRIPT "enuminit.bat")	
		ELSE ()
			SET (ENUMINIT_SCRIPT "enuminit.sh")	
		ENDIF ()

		EXECUTE_PROCESS (
			COMMAND "${PROJECT_SOURCE_DIR}/share/scripts/${ENUMINIT_SCRIPT}"
			WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
			RESULT_VARIABLE ENUMINIT_DONE
		)

		MESSAGE (STATUS "Enuminit done.")

	ELSE ()

		MESSAGE (STATUS "Enuminit skipped.")

	ENDIF ()

ENDMACRO()
