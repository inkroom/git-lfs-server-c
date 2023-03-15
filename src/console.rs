
#[macro_export]
macro_rules! s_debug {
    // debug!(target: "my_target", key1 = 42, key2 = true; "a {} event", "log")
    // debug!(target: "my_target", "a {} event", "log")
    // (target: $target:expr, $($arg:tt)+) => (log!(target: $target, $crate::Level::Debug, $($arg)+));

    // debug!("a {} event", "log")
    ($($arg:tt)+) => (

        #[cfg(feature = "plog")]
        log::debug!($($arg)+);
        #[cfg(not(feature = "plog"))]
        println!($($arg)+);
    )
}

#[macro_export]
macro_rules! s_error {
    // debug!(target: "my_target", key1 = 42, key2 = true; "a {} event", "log")
    // debug!(target: "my_target", "a {} event", "log")
    // (target: $target:expr, $($arg:tt)+) => (log!(target: $target, $crate::Level::Debug, $($arg)+));

    // debug!("a {} event", "log")
    ($($arg:tt)+) => (

        #[cfg(feature = "plog")]
        log::error!($($arg)+);
        #[cfg(not(feature = "plog"))]
        println!($($arg)+);
    )
}

#[macro_export]
macro_rules! s_info {
    // debug!(target: "my_target", key1 = 42, key2 = true; "a {} event", "log")
    // debug!(target: "my_target", "a {} event", "log")
    // (target: $target:expr, $($arg:tt)+) => (log!(target: $target, $crate::Level::Debug, $($arg)+));

    // debug!("a {} event", "log")
    ($($arg:tt)+) => (

        #[cfg(feature = "plog")]
        log::info!($($arg)+);
        #[cfg(not(feature = "plog"))]
        println!($($arg)+);
    )
}

#[macro_export]
macro_rules! s_warn {
    // debug!(target: "my_target", key1 = 42, key2 = true; "a {} event", "log")
    // debug!(target: "my_target", "a {} event", "log")
    // (target: $target:expr, $($arg:tt)+) => (log!(target: $target, $crate::Level::Debug, $($arg)+));

    // debug!("a {} event", "log")
    ($($arg:tt)+) => (

        #[cfg(feature = "plog")]
        log::warn!($($arg)+);
        #[cfg(not(feature = "plog"))]
        println!($($arg)+);
    )
}

