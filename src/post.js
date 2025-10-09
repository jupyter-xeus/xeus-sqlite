
if (!('wasmTable' in Module)) {
    Module['wasmTable'] = wasmTable
}

Module['FS'] = FS
Module['PATH'] = PATH
Module['LDSO'] = LDSO
Module['getDylinkMetadata'] = getDylinkMetadata
Module['loadDynamicLibrary'] = loadDynamicLibrary
