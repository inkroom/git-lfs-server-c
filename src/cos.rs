use crypto::digest::Digest;
use crypto::hmac::Hmac;
use crypto::mac::Mac;
use std::time::SystemTime;

use crypto::sha1::Sha1;

static HEX_TABLE: [char; 16] = [
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
];

pub struct CosClient {
    end_point: String,
    app_secert: String,
    app_id: String,
}

impl CosClient {
    ///
    /// 从环境变量中构建
    ///
    /// # Errors
    ///
    /// 没有COS_ENDPOINT或者没有COS_APP_SECERT 环境变量
    ///
    pub fn new() -> CosClient {
        let endpoint = std::env::var("COS_ENDPOINT").expect("需要 COS_ENDPOINT 环境变量");
        let bucket = std::env::var("COS_BUCKET").expect("需要 COS_ENDPOINT 环境变量");
        let app_secert = std::env::var("COS_APP_SECERT").expect("需要 COS_APP_SECERT 环境变量");
        let app_id = std::env::var("COS_APP_ID").expect("需要 COS_APP_ID 环境变量");
        CosClient {
            end_point: format!("https://{bucket}.{endpoint}"),
            app_secert,
            app_id,
        }
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

impl CosClient {
    pub fn generate_presigned_url(&self, key: &str, expiration: u64) -> String {
        let host = &self.end_point;

        if let Ok(time) = SystemTime::now().duration_since(SystemTime::UNIX_EPOCH) {
            let key_time = format!("{};{}", time.as_secs(), time.as_secs() + expiration);
            let sign_key = sign(&self.app_secert, &key_time);

            let from_str = format!("put\n/{key}\n\n\n");

            let mut sha1 = Sha1::new();
            sha1.input_str(&from_str);
            let hash_from_str = sha1.result_str(); // 进行 sha1 Hex hash

            let str_to_sign = format!("sha1\n{key_time}\n{hash_from_str}\n");

            let sign = sign(&sign_key, &str_to_sign);

            let authoriation_str = format!("q-sign-algorithm=sha1&q-ak={}&q-sign-time={key_time}&q-key-time={key_time}&q-header-list=&q-url-param-list=&q-signature={sign}",self.app_id);

            let mut res = String::new();
            res.push_str(&format!("{host}/{key}?{authoriation_str}"));

            return res;
        }
        panic!("app error");
    }

    pub fn get_object_url(&self, key: &str) -> String {
        return format!("{}/{}", &self.end_point, key);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn generate_presigned_url_test() {
        let setting = CosClient::new();
        setting.generate_presigned_url("123", 3600);
    }
}
