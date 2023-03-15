use crypto::digest::Digest;
use crypto::hmac::Hmac;
use crypto::mac::Mac;
use std::time::SystemTime;

use crypto::sha1::Sha1;

use crate::s_debug;

static HEX_TABLE: [char; 16] = [
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
];

pub struct CosClient {
    region: String,
    app_secert: String,
    app_id: String,
    bucket_id: String,
}

impl CosClient {
    ///
    /// 从环境变量中构建
    ///
    /// # Errors
    ///
    /// 没有COS_REGION或者没有COS_APP_SECERT 环境变量
    ///
    pub fn new() -> CosClient {
        let region = std::env::var("COS_REGION").expect("需要 COS_REGION 环境变量");
        let app_secert = std::env::var("COS_APP_SECERT").expect("需要 COS_APP_SECERT 环境变量");
        let app_id = std::env::var("COS_APP_ID").expect("需要 COS_APP_ID 环境变量");
        let bucket_id = std::env::var("COS_BUCKET_ID").expect("需要 COS_BUCKET_ID 环境变量");

        CosClient {
            region,
            app_secert,
            app_id,
            bucket_id,
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
    pub fn generate_presigned_url(&self, bucket: &str, key: &str, expiration: u64) -> String {
        let host = format!(
            "https://{}-{}.cos.ap-{}.myqcloud.com",
            bucket, self.bucket_id, self.region
        );
        let mut res = String::new();
        let authoriation_str = self.sign("put", key, expiration);
        res.push_str(&format!("{host}/{key}?{authoriation_str}"));
        s_debug!("res = [{res}]");
        return res;
    }

    fn sign(&self, method: &str, uri: &str, expiration: u64) -> String {
        if let Ok(time) = SystemTime::now().duration_since(SystemTime::UNIX_EPOCH) {
            let key_time = format!("{};{}", time.as_secs(), time.as_secs() + expiration);
            // let key_time = "1678672161;1678675761";
            let sign_key = sign(&self.app_secert, &key_time);
            let from_str = format!("{method}\n/{uri}\n\n\n");
            let mut sha1 = Sha1::new();
            sha1.input_str(&from_str);
            let hash_from_str = sha1.result_str(); // 进行 sha1 Hex hash

            let str_to_sign = format!("sha1\n{key_time}\n{hash_from_str}\n");
            let sign = sign(&sign_key, &str_to_sign);

            let authoriation_str = format!("q-sign-algorithm=sha1&q-ak={}&q-sign-time={key_time}&q-key-time={key_time}&q-header-list=&q-url-param-list=&q-signature={sign}",self.app_id);

            return authoriation_str;
        }
        panic!("app error");
    }

    pub fn get_object_url(&self, bucket: &str, key: &str) -> String {
        return format!(
            "https://{}-{}.cos.ap-{}.myqcloud.com/{}",
            bucket, self.bucket_id, &self.region, key
        );
    }
    #[cfg(feature = "bucket")]
    pub fn bucket_exists(&self, bucket: &str) -> bool {
        use crate::s_error;

        let url = format!(
            "https://{bucket}-{}.cos.ap-{}.myqcloud.com",
            self.bucket_id, self.region
        );

        let r = self.sign("head", "", 3600);

        s_debug!("sign=[{r}]");

        let client = reqwest::blocking::Client::new();
        match client.head(url).header("Authorization", &r).send() {
            Ok(res) => {
                s_debug!("status=[{}]", res.status());

                res.status().as_u16() == 200
            }
            Err(e) => {
                s_error!("{}", e);
                false
            }
        }
    }
    #[cfg(feature = "bucket")]
    pub fn bucket_create(&self, bucket: &str) -> bool {
        use crate::s_error;

        let url = format!(
            "https://{bucket}-{}.cos.ap-{}.myqcloud.com",
            self.bucket_id, self.region
        );

        let r = self.sign("put", "", 3600);
        s_debug!("sign=[{r}]");

        let client = reqwest::blocking::Client::new();
        match client
            .put(url)
            .header("x-cos-acl", "public-read")
            .header("Authorization", &r)
            .send()
        {
            Ok(res) => {
                s_debug!("status=[{}]", res.status());

                res.status().as_u16() == 200
            }
            Err(e) => {
                s_error!("{}", e);
                false
            }
        }
    }
    #[cfg(feature = "bucket")]
    pub fn bucket_delete(&self, bucket: &str) -> bool {
        let url = format!(
            "https://{bucket}-{}.cos.ap-{}.myqcloud.com",
            self.bucket_id, self.region
        );

        let r = self.sign("delete", "", 3600);
        s_debug!("sign=[{r}]");

        let client = reqwest::blocking::Client::new();
        match client.put(url).header("Authorization", &r).send() {
            Ok(res) => {
                s_debug!("status=[{}]", res.status());

                res.status().as_u16() == 200
            }
            Err(e) => {
                s_debug!("{}", e);
                false
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    #[cfg(feature = "bucket")]
    fn bucket_create() {
        let setting = CosClient::new();
        setting.bucket_delete("rust");
        std::thread::sleep(std::time::Duration::from_secs(10));
        assert!(setting.bucket_create("rust"));
        std::thread::sleep(std::time::Duration::from_secs(10));
        assert!(setting.bucket_exists("rust"));
        std::thread::sleep(std::time::Duration::from_secs(10));
    }

    #[test]
    #[cfg(feature = "bucket")]
    fn bucket_exists() {
        let setting = CosClient::new();
        assert!(setting.bucket_exists("image"));
        assert!(!setting.bucket_exists("image2"));
    }

    #[test]
    fn generate_presigned_url_test() {
        let setting = CosClient::new();
        println!("{}", setting.generate_presigned_url("image", "123", 3600));
    }
}
