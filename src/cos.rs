use crypto::digest::Digest;
use crypto::hmac::Hmac;
use crypto::mac::Mac;
use std::collections::HashMap;
use std::time::SystemTime;

use crypto::sha1::Sha1;

static HEX_TABLE: [char; 16] = [
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
];

pub struct CosSetting{
    end_point: String,
    app_secert: String,
}

impl CosSetting {
    ///
    /// 从环境变量中构建
    /// 
    /// # Errors
    /// 
    /// 没有COS_ENDPOINT或者没有COS_APP_SECERT 环境变量
    /// 
    pub fn new()-> CosSetting{
        let endpoint = std::env::var("COS_ENDPOINT").expect("需要 COS_ENDPOINT 环境变量");
        let app_secert = std::env::var("COS_APP_SECERT").expect("需要 COS_APP_SECERT 环境变量");
        CosSetting {end_point:endpoint,app_secert}
    }
}

fn to_hex(data: &[u8]) -> String {
    let len = data.len();
    let mut res = String::with_capacity(len * 2);

    for i in 0..len {
        res.push(HEX_TABLE[usize::from(data[i] >> 4)]);
        res.push(HEX_TABLE[usize::from(data[i] & 0x0F)]);
    }
    res
}

fn sign(key: &str, key_time: &str) -> String {
    let mut mac = Hmac::new(Sha1::new(), key.as_bytes());
    mac.input(key_time.as_bytes());

    let res = mac.result();

    return to_hex(res.code());
}

pub fn generate_presigned_url(setting:&CosSetting,
    key: &str,
    expiration: u64,
) -> String {
    let mut sign_headers = HashMap::new();

    let host = &setting.end_point;

    sign_headers.insert("host", &host); // 可能还有其他请求头参与签名，暂时不管那些

    if let Ok(time) = SystemTime::now().duration_since(SystemTime::UNIX_EPOCH) {

        let key_time = format!("{};{}", time.as_secs(), time.as_secs() + expiration);
        let sign_key = sign(&setting.app_secert, &key_time);

        let from_str = format!("put\n{key}\n\nhost={}\n", &host);

        let mut sha1 = Sha1::new();
        sha1.input_str(&from_str);
        let hash_from_str = sha1.result_str(); // 进行 sha1 Hex hash

        let str_to_sign = format!("sha1\n{key_time}\n{hash_from_str}\n");

        let sign = sign(&sign_key, &str_to_sign);

        let authoriation_str = format!("q-sign-algorithm=sha1&q-ak=AKIDJTQAh27xa0C907B2e3KODpERQBhz2pUx&q-sign-time={key_time}&q-key-time={key_time}&q-header-list=host&q-url-param-list=&q-signature={sign}");

        let mut res = String::from("https://");
        res.push_str(&format!("{host}/{key}?{authoriation_str}"));

        return res;
    }
    panic!("app error");
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn generate_presigned_url_test() {
        let setting = CosSetting::new();
        generate_presigned_url(&setting,"123",3600);
    }
}
