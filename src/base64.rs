use std::string::FromUtf8Error;

pub fn decode(value: &str) -> Result<String, FromUtf8Error> {
    let table = [
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
        'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
        'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1',
        '2', '3', '4', '5', '6', '7', '8', '9', '+', '/',
    ].to_vec();

    // 字符串 转 字节数组
    let bytes = value.trim_matches(|c| c == '=').chars();
    let mut res: Vec<u8> = Vec::new();
    let mut t = 0;

    let mut buffer = 0;

    for c in bytes {
        let index = find_index(&table, &c);
        buffer = buffer << 6;
        buffer = buffer | index;

        if t == 3 {
            // 凑够 24 bit，也就是处理了一组4个字符

            // 按 8 bit 一组 重新生成byte
            res.push((buffer >> 16) as u8);
            res.push(((buffer & 0xff00) >> 8) as u8);
            res.push((buffer & 0xff) as u8);

            t = -1;
            buffer = 0;
        }

        t = t + 1;
    }

    if buffer != 0 {
        //说明还有剩余的字符不能凑成一组，需要单独处理
        if t == 2 {
            //还有12bit 8 + 4 最后4bit是补位的0，不需要
            buffer = buffer >> 4;
            res.push((buffer & 0xff) as u8);
        } else if t == 3 {
            //还有18bit，8 + 8 + 2 最后2bit是补位的0，不需要
            buffer = buffer >> 2;
            res.push(((buffer & 0xff00) >> 8) as u8);
            res.push((buffer & 0xff) as u8);
        }
    }

    return String::from_utf8(res);
}

fn find_index(array: &Vec<char>, char: &char) -> i32 {
    if char == &'-' {
        return 62;
    } else if char == &'_' {
        return 63;
    }

    let mut index = -1;
    for a in array {
        index += 1;
        if a == char {
            return index;
        }
    }
    return index;
}

#[cfg(test)]
mod tests {
    use std::collections::HashMap;

    use super::*;

    #[test]
    fn decode_test() {
        let mut map = HashMap::new();
        map.insert("15742115", "MTU3NDIxMTU=");
        map.insert("sdssd3ds", "c2Rzc2QzZHM=");
        map.insert("l;;32.sd", "bDs7MzIuc2Q=");
        map.insert("l;;32.34d", "bDs7MzIuMzRk");
        map.insert("1023s.1353", "MTAyM3MuMTM1Mw==");
        map.insert("dsfasd23", "ZHNmYXNkMjM=");
        map.insert("中文", "5Lit5paH");
        map.insert("中英混杂32.kl23", "5Lit6Iux5re35p2CMzIua2wyMw==");

        for m in map {
            println!("m0 = {},m2={}", m.0, m.1);
            if let Ok(base) = decode(m.1) {
                assert_eq!(m.0, base);
            } else {
                panic!("base 64 error");
            }
        }
    }
}
