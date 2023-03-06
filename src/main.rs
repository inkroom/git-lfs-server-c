use std::{
    io::{BufRead, BufReader, Read, Write},
    net::TcpListener,
};

use cos::CosSetting;
use json;
pub mod base64;
pub mod thread;
use thread::ThreadPool;
pub mod cos;
fn main() {

    // 获取账号密码

    let username = std::env::var("LFS_USERNAME").expect("需要 LFS_USERNAME 环境变量 ");
    let password = std::env::var("LFS_PASSWORD").expect("需要 LFS_PASSWORD 环境变量 ");

    let setting = cos::CosSetting::new();

    let listener = TcpListener::bind("127.0.0.1:8998").unwrap();
    // let pool = ThreadPool::new(4);
    for stream in listener.incoming() {
        // pool.execute(|| {
        handle_stream(stream.unwrap(), &setting, (&username, &password));
        // });
        println!("connect established");
    }
}
fn write_401(mut stream: std::net::TcpStream) {
    let response = "Unauthorized";
    let length = response.len();
    let r = format!("HTTP/1.1 401 Unauthorized\r\nContent-Length: {length}\r\nContent-Type: application/vnd.git-lfs+json\r\n\r\n{response}");
    stream.write_all(r.as_bytes()).unwrap();
}

fn auth(basic: &str, account: (&str, &str)) -> bool {
    return match base64::decode(basic) {
        Ok(s) => {
            let s2: Vec<_> = s.split(":").collect();
            s2.len() == 2 && s2[0] == account.0 && s2[1] == account.1
        }
        Err(_) => false,
    };
}

fn handle_stream(mut stream: std::net::TcpStream, setting: &CosSetting, account: (&str, &str)) {
    let mut reader = BufReader::new(&mut stream);
    let mut line = String::new();
    let _len = reader.read_line(&mut line).unwrap();

    line.clear();

    let mut body_length = 0;

    let mut basic = String::new();
    loop {
        line.clear();
        if let Ok(_len) = reader.read_line(&mut line) {
            let s = line.trim();

            if s.is_empty() {
                break;
            }
            if s.starts_with("Content-Length: ") {
                body_length = match s[16..].parse::<u64>() {
                    Ok(l) => l,
                    Err(_) => 0,
                }
            } else if s.starts_with("Authorization: Basic ") {
                basic.push_str(&s[21..]);
            }
        } else {
            break;
        }
    }

    let mut response_body = json::JsonValue::new_object();

    if body_length > 0 {
        // 读取body
        let mut chunk = reader.take(body_length);
        let mut body = vec![];

        match chunk.read_to_end(&mut body) {
            Ok(_) => {
                let body_str = std::str::from_utf8(&mut body).unwrap();
                println!("body=[{}]", body_str);

                if let Ok(json) = json::parse(&body_str) {
                    let mut objects = json::JsonValue::new_array();

                    // println!("json [{:#?}] [{}] [{}] ",json["objects"],json["operation"],json["operation"] == "upload");

                    if json["operation"] == "upload" {
                        // 认证
                        if basic.is_empty() || !auth(&basic, account) {
                            // 401
                            write_401(stream);
                            return;
                        }

                        response_body["transfer"] = "basic".into();
                        response_body["hash_algo"] = "sha256".into();

                        for oid in json["objects"].members() {
                            let mut object = json::JsonValue::new_object();
                            object["oid"] = oid["oid"].clone(); // 直接赋值，因为 oid 会在循环结束后回收，所以编译通不过，只能直接clone
                            object["size"] = oid["size"].clone();
                            object["authenticated"] = true.into();

                            let mut actions = json::JsonValue::new_array();
                            let mut action = json::JsonValue::new_object();
                            let mut upload = json::JsonValue::new_object();
                            upload["href"] = json::JsonValue::String(cos::generate_presigned_url(
                                setting,
                                &oid["oid"].to_string(),
                                3600,
                            ));

                            action["upload"] = upload;

                            actions.push(action).unwrap();
                            object["actions"] = actions;
                            objects.push(object).unwrap();
                        }
                    } else if json["operation"] == "download" {
                        for oid in json["objects"].members() {
                            let mut object = json::JsonValue::new_object();
                            object["oid"] = oid["oid"].clone(); // 直接赋值，因为 oid 会在循环结束后回收，所以编译通不过，只能直接clone
                            object["size"] = oid["size"].clone();
                            object["authenticated"] = true.into();

                            let mut actions = json::JsonValue::new_array();
                            let mut action = json::JsonValue::new_object();
                            let mut download = json::JsonValue::new_object();
                            download["href"] = json::JsonValue::String(
                                cos::generate_presigned_url(setting, &oid["oid"].to_string(), 3600),
                            );
                            action["download"] = download;
                            actions.push(action).unwrap();
                            object["actions"] = actions;
                            objects.push(object).unwrap();
                        }
                    }

                    response_body["objects"] = objects;
                }
            }
            Err(_) => {}
        }
    }

    let mut response = String::from("inkbox");

    if body_length != 0 {
        response.clear();
        response.push_str(&response_body.to_string());
    }

    let length = response.len();

    let r = format!("HTTP/1.1 200 OK\r\nContent-Length: {length}\r\nContent-Type: application/vnd.git-lfs+json\r\n\r\n{response}");

    stream.write_all(r.as_bytes()).unwrap();
    // stream.write_all(response.as_bytes()).unwrap();
    // stream.write_all("HTTP/1.1 200 OK\r\nContent-Length: 6\r\n\r\ninkbox".as_bytes()).unwrap();
}
