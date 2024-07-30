MessageIdTypedef=DWORD

SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
    Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
    Warning=0x2:STATUS_SEVERITY_WARNING
    Error=0x3:STATUS_SEVERITY_ERROR
    )


FacilityNames=(System=0x0:FACILITY_SYSTEM
    Runtime=0x2:FACILITY_RUNTIME
    Stubs=0x3:FACILITY_STUBS
    Io=0x4:FACILITY_IO_ERROR_CODE
)

LanguageNames=(English=0x409:MSG00409)

; // The following are message definitions.

MessageIdTypedef=DWORD

MessageId=0x1
Severity=Error
Facility=Runtime
SymbolicName=WL_ERROR
Language=English
%2
.

MessageId=0x2
Severity=Warning
Facility=Runtime
SymbolicName=WL_WARNING
Language=English
%2
.

MessageId=0x3
Severity=Informational
Facility=Runtime
SymbolicName=WL_INFO
Language=English
%2
.

; // A message file must end with a period on its own line
; // followed by a blank line.
