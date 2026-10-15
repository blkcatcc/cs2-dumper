// build 14191

pub const cs2_dumper = struct {
    pub const schemas = struct {
        pub const libhost_so = struct {
            pub const CAnimScriptBase = struct {
                pub const m_bIsValid: usize = 0x8; // bool
            };
            pub const EmptyTestScript = struct {
                pub const m_hTest: usize = 0x10; // CAnimScriptParam<float32>
            };
        };
    };
};
