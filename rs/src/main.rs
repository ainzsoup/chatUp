use std::io::{stdout, Write};

use actix_web::{get, web, App, HttpServer, Responder};
use config::Config;
use dotenv::dotenv;
use serde::Deserialize;

#[derive(Debug, Deserialize)]
pub struct Auth {
    pub code: String,
    pub state: Option<u32>,
}

#[derive(Debug, Deserialize)]
pub struct Token {
    pub access_token: String,
    pub token_type: String,
    pub expires_in: u32,
    pub refresh_token: Option<String>,
}

#[derive(Debug, Deserialize)]
pub struct Me {
    pub login: String,
}

#[get("/auth")]
async fn auth(info: web::Query<Auth>, config: web::Data<EnvConfig>) -> impl Responder {
    let code = &info.code;
    let state = &info.state;
    let client = reqwest::Client::new();

    let token: Token = client
        .post(&config.token_url)
        .form(&[
            ("grant_type", "authorization_code"),
            ("client_id", &config.client_id),
            ("client_secret", &config.client_secret),
            ("code", code),
            ("redirect_uri", "http://10.12.12.1:8080/auth"),
        ])
        .send()
        .await
        .unwrap()
        .json()
        .await
        .unwrap();

    let client = reqwest::Client::new();

    let me: Me = client
        .get(&config.me_url)
        .bearer_auth(&token.access_token)
        .send()
        .await
        .unwrap()
        .json()
        .await
        .unwrap();
    // print to the standard output 4byte of state + 4 bytes the size + size byte login
    print!(
        "{:08x}{:08x}{}",
        state.unwrap_or(0),
        me.login.len(),
        me.login
    );
    // flush the standard output
    stdout().flush().unwrap();
    "Done!"
}

#[derive(Debug, Default, Deserialize, Clone)]
pub struct EnvConfig {
    pub server_addr: String,
    pub token_url: String,
    pub client_id: String,
    pub client_secret: String,
    pub me_url: String,
}

#[tokio::main]
async fn main() -> std::io::Result<()> {
    dotenv().ok();

    let config_ = Config::builder()
        .add_source(::config::Environment::default())
        .build()
        .unwrap();

    let config: &EnvConfig = Box::leak(Box::new(config_.try_deserialize().unwrap()));
    let server_addr = config.server_addr.clone();
    HttpServer::new(|| {
        let data_config = web::Data::new(config.clone());
        App::new().app_data(data_config).service(auth)
    })
    .bind(server_addr)?
    .run()
    .await
}
