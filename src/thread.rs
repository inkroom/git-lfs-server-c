
///
/// 线程池实现
/// 
/// 暂未使用

use std::{
    sync::{mpsc, Arc, Mutex},
    thread,
};
pub struct ThreadPool{
    workers : Vec<Worker>,
    sender: Option<std::sync::mpsc::Sender<Job>>
    // threads: Vec<std::thread::JoinHandle<()>>,
}

impl ThreadPool{
    ///
    /// 创建线程池
    /// 
    /// # Panics
    /// 
    /// size不对
    /// 
    pub fn new(size: usize)->ThreadPool {
        assert!(size > 0);

        let (sender,receiver) = std::sync::mpsc::channel();

        let receiver = Arc::new(Mutex::new(receiver));

        // let mut thraeds = Vec::with_capacity(size);
        let mut workers = Vec::with_capacity(size);
        for t in 0..size {
            workers.push(Worker::new(t,Arc::clone(&receiver)));
        }

        ThreadPool {workers,sender:Some(sender) }

    }

    pub fn execute<F>(&self, f:F)
    where  F: FnOnce() + Send + 'static{
        let job = Box::new(f);

        self.sender.as_ref().unwrap().send(job).unwrap();
    }
}

impl Drop for ThreadPool {
    fn drop(&mut self){
        drop(self.sender.take());
        for worker in &mut self.workers {
                        println!("Shutting down worker {}", worker.id);

            if let Some(thread) = worker.thread.take(){
                thread.join().unwrap();
            }
        }
    }
}


struct Worker {
    id:usize,
    thread: Option< thread::JoinHandle<()>>,
}

impl Worker{

    fn new(id:usize,receiver: Arc<Mutex< mpsc::Receiver<Job>>>) -> Worker{
        let thread = std::thread::spawn(move || loop{
            let job  = receiver.lock().unwrap().recv();
            match job {
                Ok(r)=>{
                    println!("woker {id} get task ");
                    r();
                }
                Err(_)=>{
                    println!("Worker {id} disconnected; shutting down.");
                    break;
                }
            }
        });
         Worker {id,thread:Some(thread) }
        }
}


type Job = Box<dyn FnOnce() + Send + 'static>;