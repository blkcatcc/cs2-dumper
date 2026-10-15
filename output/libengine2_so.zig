// build 14191

pub const cs2_dumper = struct {
    pub const schemas = struct {
        pub const libengine_2_so = struct {
            pub const EntityDormancyType_t = enum(u32) {
                ENTITY_NOT_DORMANT = 0x0,
                ENTITY_DORMANT = 0x1,
                ENTITY_SUSPENDED = 0x2
            };
            pub const EntityIOTargetType_t = enum(u32) {
                ENTITY_IO_TARGET_INVALID = 0xFFFFFFFF,
                ENTITY_IO_TARGET_ENTITYNAME = 0x2,
                ENTITY_IO_TARGET_EHANDLE = 0x6,
                ENTITY_IO_TARGET_ENTITYNAME_OR_CLASSNAME = 0x7
            };
            pub const CEmptyEntityInstance = struct {
            };
            pub const CEntityAttributeTable = struct {
                pub const m_Attributes: usize = 0x0; // CUtlOrderedMap<CUtlStringTokenNoRegistration,Attribute_t>
                pub const m_Names: usize = 0x28; // CUtlOrderedMap<CUtlStringTokenNoRegistration,CUtlString>
            };
            pub const CEntityComponent = struct {
            };
            pub const CEntityComponentHelper = struct {
                pub const m_flags: usize = 0x8; // uint32
                pub const m_pInfo: usize = 0x10; // EntComponentInfo_t*
                pub const m_nPriority: usize = 0x18; // int32
                pub const m_pNext: usize = 0x20; // CEntityComponentHelper*
            };
            pub const CEntityIOOutput = struct {
            };
            pub const CEntityIdentity = struct {
                pub const m_nameStringTableIndex: usize = 0x14; // int32
                pub const m_name: usize = 0x18; // CUtlSymbolLarge
                pub const m_designerName: usize = 0x20; // CUtlSymbolLarge
                pub const m_flags: usize = 0x30; // uint32
                pub const m_worldGroupId: usize = 0x38; // WorldGroupId_t
                pub const m_fDataObjectTypes: usize = 0x3C; // uint32
                pub const m_PathIndex: usize = 0x40; // ChangeAccessorFieldPathIndex_t
                pub const m_pAttributes: usize = 0x48; // CEntityAttributeTable*
                pub const m_pPrev: usize = 0x50; // CEntityIdentity*
                pub const m_pNext: usize = 0x58; // CEntityIdentity*
                pub const m_pPrevByClass: usize = 0x60; // CEntityIdentity*
                pub const m_pNextByClass: usize = 0x68; // CEntityIdentity*
            };
            pub const CEntityInstance = struct {
                pub const m_iszPrivateVScripts: usize = 0x8; // CUtlSymbolLarge
                pub const m_pEntity: usize = 0x10; // CEntityIdentity*
                pub const m_CScriptComponent: usize = 0x28; // CScriptComponent*
            };
            pub const CEntityKeyValues = struct {
            };
            pub const CNetworkVarChainer = struct {
                pub const m_PathIndex: usize = 0x20; // ChangeAccessorFieldPathIndex_t
            };
            pub const CScriptComponent = struct {
                pub const m_scriptClassName: usize = 0x30; // CUtlSymbolLarge
            };
            pub const CVariantDefaultAllocator = struct {
            };
            pub const EngineLoopState_t = struct {
                pub const m_nPlatWindowWidth: usize = 0x18; // int32
                pub const m_nPlatWindowHeight: usize = 0x1C; // int32
                pub const m_nRenderWidth: usize = 0x20; // int32
                pub const m_nRenderHeight: usize = 0x24; // int32
            };
            pub const EntComponentInfo_t = struct {
                pub const m_pName: usize = 0x0; // char*
                pub const m_pCPPClassname: usize = 0x8; // char*
                pub const m_pNetworkDataReferencedDescription: usize = 0x10; // char*
                pub const m_pNetworkDataReferencedPtrPropDescription: usize = 0x18; // char*
                pub const m_nRuntimeIndex: usize = 0x20; // int32
                pub const m_nFlags: usize = 0x24; // uint32
                pub const m_pBaseClassComponentHelper: usize = 0x58; // CEntityComponentHelper*
            };
            pub const EntityIOQueuePrioritizedEvent_t = struct {
                pub const m_flFireTime: usize = 0x4; // GameTime_t
                pub const m_targetType: usize = 0x8; // EntityIOTargetType_t
                pub const m_pTarget: usize = 0x10; // CUtlSymbolLarge
                pub const m_pTargetInput: usize = 0x18; // CUtlSymbolLarge
                pub const m_hActivator: usize = 0x20; // CEntityHandle
                pub const m_hCaller: usize = 0x24; // CEntityHandle
                pub const m_hEntTarget: usize = 0x28; // CEntityHandle
                pub const m_variantValue: usize = 0x30; // CVariantBase<CVariantDefaultAllocator>
                pub const m_PulseArguments: usize = 0x40; // CPulseArgumentPack
                pub const m_paramMap: usize = 0xD0; // CPulseInputParamMap
            };
            pub const EventAdvanceTick_t = struct {
                pub const m_nCurrentTick: usize = 0x2C; // int32
                pub const m_nCurrentTickThisFrame: usize = 0x30; // int32
                pub const m_nTotalTicksThisFrame: usize = 0x34; // int32
                pub const m_nTotalTicks: usize = 0x38; // int32
            };
            pub const EventAppShutdown_t = struct {
                pub const m_nDummy0: usize = 0x0; // int32
            };
            pub const EventBugBugComplete_t = struct {
                pub const m_pPayload: usize = 0x0; // EventBugBug_t*
            };
            pub const EventBugBug_t = struct {
            };
            pub const EventClientAdvanceNonRenderedFrame_t = struct {
            };
            pub const EventClientAdvanceTick_t = struct {
            };
            pub const EventClientFrameSimulate_t = struct {
                pub const m_LoopState: usize = 0x0; // EngineLoopState_t
                pub const m_flRealTime: usize = 0x28; // float32
                pub const m_flFrameTime: usize = 0x2C; // float32
                pub const m_bScheduleSendTickPacket: usize = 0x30; // bool
            };
            pub const EventClientOutput_t = struct {
                pub const m_LoopState: usize = 0x0; // EngineLoopState_t
                pub const m_flRenderTime: usize = 0x28; // float32
                pub const m_flRealTime: usize = 0x2C; // float32
                pub const m_flRenderFrameTimeUnbounded: usize = 0x30; // float32
                pub const m_bRenderOnly: usize = 0x34; // bool
            };
            pub const EventClientPauseSimulate_t = struct {
            };
            pub const EventClientPollInput_t = struct {
                pub const m_LoopState: usize = 0x0; // EngineLoopState_t
                pub const m_flRealTime: usize = 0x28; // float32
            };
            pub const EventClientPollNetworking_t = struct {
                pub const m_nTickCount: usize = 0x0; // int32
            };
            pub const EventClientPostAdvanceTick_t = struct {
            };
            pub const EventClientPostOutput_t = struct {
                pub const m_LoopState: usize = 0x0; // EngineLoopState_t
                pub const m_flRenderTime: usize = 0x28; // float64
                pub const m_flRenderFrameTime: usize = 0x30; // float32
                pub const m_flRenderFrameTimeUnbounded: usize = 0x34; // float32
                pub const m_bRenderOnly: usize = 0x38; // bool
            };
            pub const EventClientPostSimulate_t = struct {
            };
            pub const EventClientPreOutputParallelWithServer_t = struct {
            };
            pub const EventClientPreOutput_t = struct {
                pub const m_LoopState: usize = 0x0; // EngineLoopState_t
                pub const m_flRenderTime: usize = 0x28; // float64
                pub const m_flRenderFrameTime: usize = 0x30; // float64
                pub const m_flRenderFrameTimeUnbounded: usize = 0x38; // float64
                pub const m_flRealTime: usize = 0x40; // float32
                pub const m_bRenderOnly: usize = 0x44; // bool
            };
            pub const EventClientPreSimulate_t = struct {
            };
            pub const EventClientProcessGameInput_t = struct {
                pub const m_LoopState: usize = 0x0; // EngineLoopState_t
                pub const m_flRealTime: usize = 0x28; // float32
                pub const m_flFrameTime: usize = 0x2C; // float32
            };
            pub const EventClientProcessInput_t = struct {
                pub const m_LoopState: usize = 0x0; // EngineLoopState_t
                pub const m_flRealTime: usize = 0x28; // float32
                pub const m_flTickInterval: usize = 0x2C; // float32
                pub const m_flTickStartTime: usize = 0x30; // float64
            };
            pub const EventClientProcessNetworking_t = struct {
                pub const m_nTickCount: usize = 0x0; // int32
            };
            pub const EventClientSceneSystemThreadStateChange_t = struct {
                pub const m_bThreadsActive: usize = 0x0; // bool
            };
            pub const EventClientSimulate_t = struct {
            };
            pub const EventFrameBoundary_t = struct {
                pub const m_flFrameTime: usize = 0x0; // float32
            };
            pub const EventModInitialized_t = struct {
            };
            pub const EventPostAdvanceTick_t = struct {
                pub const m_nCurrentTick: usize = 0x2C; // int32
                pub const m_nCurrentTickThisFrame: usize = 0x30; // int32
                pub const m_nTotalTicksThisFrame: usize = 0x34; // int32
                pub const m_nTotalTicks: usize = 0x38; // int32
            };
            pub const EventPostDataUpdate_t = struct {
                pub const m_nCount: usize = 0x0; // int32
            };
            pub const EventPreDataUpdate_t = struct {
                pub const m_nCount: usize = 0x0; // int32
            };
            pub const EventProfileStorageAvailable_t = struct {
                pub const m_nSplitScreenSlot: usize = 0x0; // CSplitScreenSlot
            };
            pub const EventServerAdvanceTick_t = struct {
            };
            pub const EventServerBeginAsyncPostTickWork_t = struct {
                pub const m_bIsOncePerFrameAsyncWorkPhase: usize = 0x0; // bool
            };
            pub const EventServerBeginSimulate_t = struct {
            };
            pub const EventServerEndAsyncPostTickWork_t = struct {
            };
            pub const EventServerEndSimulate_t = struct {
                pub const m_bLastTick: usize = 0x0; // bool
            };
            pub const EventServerPollNetworking_t = struct {
            };
            pub const EventServerPostAdvanceTick_t = struct {
                pub const m_bLastTickBeforeClientUpdate: usize = 0x3C; // bool
            };
            pub const EventServerPostSimulate_t = struct {
                pub const m_bLastTickBeforeClientUpdate: usize = 0x2A; // bool
            };
            pub const EventServerProcessNetworking_t = struct {
            };
            pub const EventSetTime_t = struct {
                pub const m_LoopState: usize = 0x0; // EngineLoopState_t
                pub const m_nClientOutputFrames: usize = 0x28; // int32
                pub const m_flRealTime: usize = 0x30; // float64
                pub const m_flRenderTime: usize = 0x38; // float64
                pub const m_flRenderFrameTime: usize = 0x40; // float64
                pub const m_flRenderFrameTimeUnbounded: usize = 0x48; // float64
                pub const m_flRenderFrameTimeUnscaled: usize = 0x50; // float64
                pub const m_flTickRemainder: usize = 0x58; // float64
            };
            pub const EventSimpleLoopFrameUpdate_t = struct {
                pub const m_LoopState: usize = 0x0; // EngineLoopState_t
                pub const m_flRealTime: usize = 0x28; // float32
                pub const m_flFrameTime: usize = 0x2C; // float32
            };
            pub const EventSimulate_t = struct {
                pub const m_LoopState: usize = 0x0; // EngineLoopState_t
                pub const m_bFirstTick: usize = 0x28; // bool
                pub const m_bLastTick: usize = 0x29; // bool
            };
            pub const EventSplitScreenStateChanged_t = struct {
            };
            pub const GameTick_t = struct {
                pub const m_Value: usize = 0x0; // int32
            };
            pub const GameTime_t = struct {
                pub const m_Value: usize = 0x0; // float32
            };
        };
    };
};
