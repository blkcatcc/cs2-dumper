// build 14191

#![allow(non_upper_case_globals, non_camel_case_types, non_snake_case, unused)]

pub mod cs2_dumper {
    pub mod schemas {
        pub mod libengine_2_so {
            #[repr(u32)]
            pub enum EntityDormancyType_t {
                ENTITY_NOT_DORMANT = 0x0,
                ENTITY_DORMANT = 0x1,
                ENTITY_SUSPENDED = 0x2
            }
            #[repr(u32)]
            pub enum EntityIOTargetType_t {
                ENTITY_IO_TARGET_INVALID = u32::MAX,
                ENTITY_IO_TARGET_ENTITYNAME = 0x2,
                ENTITY_IO_TARGET_EHANDLE = 0x6,
                ENTITY_IO_TARGET_ENTITYNAME_OR_CLASSNAME = 0x7
            }
            pub mod CEmptyEntityInstance {
            }
            pub mod CEntityAttributeTable {
                pub const m_Attributes: usize = 0x0; // CUtlOrderedMap<CUtlStringTokenNoRegistration,Attribute_t>
                pub const m_Names: usize = 0x28; // CUtlOrderedMap<CUtlStringTokenNoRegistration,CUtlString>
            }
            pub mod CEntityComponent {
            }
            pub mod CEntityComponentHelper {
                pub const m_flags: usize = 0x8; // uint32
                pub const m_pInfo: usize = 0x10; // EntComponentInfo_t*
                pub const m_nPriority: usize = 0x18; // int32
                pub const m_pNext: usize = 0x20; // CEntityComponentHelper*
            }
            pub mod CEntityIOOutput {
            }
            pub mod CEntityIdentity {
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
            }
            pub mod CEntityInstance {
                pub const m_iszPrivateVScripts: usize = 0x8; // CUtlSymbolLarge
                pub const m_pEntity: usize = 0x10; // CEntityIdentity*
                pub const m_CScriptComponent: usize = 0x28; // CScriptComponent*
            }
            pub mod CEntityKeyValues {
            }
            pub mod CNetworkVarChainer {
                pub const m_PathIndex: usize = 0x20; // ChangeAccessorFieldPathIndex_t
            }
            pub mod CScriptComponent {
                pub const m_scriptClassName: usize = 0x30; // CUtlSymbolLarge
            }
            pub mod CVariantDefaultAllocator {
            }
            pub mod EngineLoopState_t {
                pub const m_nPlatWindowWidth: usize = 0x18; // int32
                pub const m_nPlatWindowHeight: usize = 0x1C; // int32
                pub const m_nRenderWidth: usize = 0x20; // int32
                pub const m_nRenderHeight: usize = 0x24; // int32
            }
            pub mod EntComponentInfo_t {
                pub const m_pName: usize = 0x0; // char*
                pub const m_pCPPClassname: usize = 0x8; // char*
                pub const m_pNetworkDataReferencedDescription: usize = 0x10; // char*
                pub const m_pNetworkDataReferencedPtrPropDescription: usize = 0x18; // char*
                pub const m_nRuntimeIndex: usize = 0x20; // int32
                pub const m_nFlags: usize = 0x24; // uint32
                pub const m_pBaseClassComponentHelper: usize = 0x58; // CEntityComponentHelper*
            }
            pub mod EntityIOQueuePrioritizedEvent_t {
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
            }
            pub mod EventAdvanceTick_t {
                pub const m_nCurrentTick: usize = 0x2C; // int32
                pub const m_nCurrentTickThisFrame: usize = 0x30; // int32
                pub const m_nTotalTicksThisFrame: usize = 0x34; // int32
                pub const m_nTotalTicks: usize = 0x38; // int32
            }
            pub mod EventAppShutdown_t {
                pub const m_nDummy0: usize = 0x0; // int32
            }
            pub mod EventBugBugComplete_t {
                pub const m_pPayload: usize = 0x0; // EventBugBug_t*
            }
            pub mod EventBugBug_t {
            }
            pub mod EventClientAdvanceNonRenderedFrame_t {
            }
            pub mod EventClientAdvanceTick_t {
            }
            pub mod EventClientFrameSimulate_t {
                pub const m_LoopState: usize = 0x0; // EngineLoopState_t
                pub const m_flRealTime: usize = 0x28; // float32
                pub const m_flFrameTime: usize = 0x2C; // float32
                pub const m_bScheduleSendTickPacket: usize = 0x30; // bool
            }
            pub mod EventClientOutput_t {
                pub const m_LoopState: usize = 0x0; // EngineLoopState_t
                pub const m_flRenderTime: usize = 0x28; // float32
                pub const m_flRealTime: usize = 0x2C; // float32
                pub const m_flRenderFrameTimeUnbounded: usize = 0x30; // float32
                pub const m_bRenderOnly: usize = 0x34; // bool
            }
            pub mod EventClientPauseSimulate_t {
            }
            pub mod EventClientPollInput_t {
                pub const m_LoopState: usize = 0x0; // EngineLoopState_t
                pub const m_flRealTime: usize = 0x28; // float32
            }
            pub mod EventClientPollNetworking_t {
                pub const m_nTickCount: usize = 0x0; // int32
            }
            pub mod EventClientPostAdvanceTick_t {
            }
            pub mod EventClientPostOutput_t {
                pub const m_LoopState: usize = 0x0; // EngineLoopState_t
                pub const m_flRenderTime: usize = 0x28; // float64
                pub const m_flRenderFrameTime: usize = 0x30; // float32
                pub const m_flRenderFrameTimeUnbounded: usize = 0x34; // float32
                pub const m_bRenderOnly: usize = 0x38; // bool
            }
            pub mod EventClientPostSimulate_t {
            }
            pub mod EventClientPreOutputParallelWithServer_t {
            }
            pub mod EventClientPreOutput_t {
                pub const m_LoopState: usize = 0x0; // EngineLoopState_t
                pub const m_flRenderTime: usize = 0x28; // float64
                pub const m_flRenderFrameTime: usize = 0x30; // float64
                pub const m_flRenderFrameTimeUnbounded: usize = 0x38; // float64
                pub const m_flRealTime: usize = 0x40; // float32
                pub const m_bRenderOnly: usize = 0x44; // bool
            }
            pub mod EventClientPreSimulate_t {
            }
            pub mod EventClientProcessGameInput_t {
                pub const m_LoopState: usize = 0x0; // EngineLoopState_t
                pub const m_flRealTime: usize = 0x28; // float32
                pub const m_flFrameTime: usize = 0x2C; // float32
            }
            pub mod EventClientProcessInput_t {
                pub const m_LoopState: usize = 0x0; // EngineLoopState_t
                pub const m_flRealTime: usize = 0x28; // float32
                pub const m_flTickInterval: usize = 0x2C; // float32
                pub const m_flTickStartTime: usize = 0x30; // float64
            }
            pub mod EventClientProcessNetworking_t {
                pub const m_nTickCount: usize = 0x0; // int32
            }
            pub mod EventClientSceneSystemThreadStateChange_t {
                pub const m_bThreadsActive: usize = 0x0; // bool
            }
            pub mod EventClientSimulate_t {
            }
            pub mod EventFrameBoundary_t {
                pub const m_flFrameTime: usize = 0x0; // float32
            }
            pub mod EventModInitialized_t {
            }
            pub mod EventPostAdvanceTick_t {
                pub const m_nCurrentTick: usize = 0x2C; // int32
                pub const m_nCurrentTickThisFrame: usize = 0x30; // int32
                pub const m_nTotalTicksThisFrame: usize = 0x34; // int32
                pub const m_nTotalTicks: usize = 0x38; // int32
            }
            pub mod EventPostDataUpdate_t {
                pub const m_nCount: usize = 0x0; // int32
            }
            pub mod EventPreDataUpdate_t {
                pub const m_nCount: usize = 0x0; // int32
            }
            pub mod EventProfileStorageAvailable_t {
                pub const m_nSplitScreenSlot: usize = 0x0; // CSplitScreenSlot
            }
            pub mod EventServerAdvanceTick_t {
            }
            pub mod EventServerBeginAsyncPostTickWork_t {
                pub const m_bIsOncePerFrameAsyncWorkPhase: usize = 0x0; // bool
            }
            pub mod EventServerBeginSimulate_t {
            }
            pub mod EventServerEndAsyncPostTickWork_t {
            }
            pub mod EventServerEndSimulate_t {
                pub const m_bLastTick: usize = 0x0; // bool
            }
            pub mod EventServerPollNetworking_t {
            }
            pub mod EventServerPostAdvanceTick_t {
                pub const m_bLastTickBeforeClientUpdate: usize = 0x3C; // bool
            }
            pub mod EventServerPostSimulate_t {
                pub const m_bLastTickBeforeClientUpdate: usize = 0x2A; // bool
            }
            pub mod EventServerProcessNetworking_t {
            }
            pub mod EventSetTime_t {
                pub const m_LoopState: usize = 0x0; // EngineLoopState_t
                pub const m_nClientOutputFrames: usize = 0x28; // int32
                pub const m_flRealTime: usize = 0x30; // float64
                pub const m_flRenderTime: usize = 0x38; // float64
                pub const m_flRenderFrameTime: usize = 0x40; // float64
                pub const m_flRenderFrameTimeUnbounded: usize = 0x48; // float64
                pub const m_flRenderFrameTimeUnscaled: usize = 0x50; // float64
                pub const m_flTickRemainder: usize = 0x58; // float64
            }
            pub mod EventSimpleLoopFrameUpdate_t {
                pub const m_LoopState: usize = 0x0; // EngineLoopState_t
                pub const m_flRealTime: usize = 0x28; // float32
                pub const m_flFrameTime: usize = 0x2C; // float32
            }
            pub mod EventSimulate_t {
                pub const m_LoopState: usize = 0x0; // EngineLoopState_t
                pub const m_bFirstTick: usize = 0x28; // bool
                pub const m_bLastTick: usize = 0x29; // bool
            }
            pub mod EventSplitScreenStateChanged_t {
            }
            pub mod GameTick_t {
                pub const m_Value: usize = 0x0; // int32
            }
            pub mod GameTime_t {
                pub const m_Value: usize = 0x0; // float32
            }
        }
    }
}
