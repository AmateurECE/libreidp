///////////////////////////////////////////////////////////////////////////////
// NAME:            main.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     libreidp entrypoint.
//
// CREATED:         02/23/2022
//
// LAST EDITED:     06/25/2022
//
// Copyright 2022, Ethan D. Twardy
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
////

use std::error::Error;
use std::fs;

use axum::{http::status::StatusCode, routing::get, Router};
use clap::Parser;
use ldap3::{Ldap, LdapConnAsync, Scope, SearchEntry};
use serde::Deserialize;
use tokio::sync::{mpsc, oneshot};
use tower_http::trace::TraceLayer;
use tracing::{event, Level};

struct IdentityProxy {
    base: String,
    ldap: Ldap,
}

impl IdentityProxy {
    pub async fn new(hostname: &str, base: String) ->
        ldap3::result::Result<Self>
    {
        let (conn, ldap) = LdapConnAsync::new(hostname).await?;
        ldap3::drive!(conn);
        Ok(Self {
            base,
            ldap
        })
    }

    pub async fn query(&mut self) -> ldap3::result::Result<String> {
        let (rs, _res) = self.ldap.search(
            &self.base,
            Scope::Subtree,
            "(objectClass=*)",
            vec!["*"]
        ).await?.success()?;

        self.ldap.unbind().await?;
        Ok(rs.into_iter()
           .map(|s| format!("{:?}\n", SearchEntry::construct(s)))
           .collect::<String>())
    }
}

// Command line arguments
#[derive(Parser, Debug)]
#[clap(author, version, about = None, long_about = None)]
struct Args {
    #[clap(short, long)]
    config_file: String,
}

// Configuration for the LDAP-side
#[derive(Default, Deserialize)]
struct LdapConfiguration {
    uri: String,
    base: String,
}

// Configuration for the HTTP-side
#[derive(Default, Deserialize)]
struct HttpConfiguration {
    address: String,
}

// Serializable configuration object
#[derive(Default, Deserialize)]
struct Configuration {
    ldap: LdapConfiguration,
    http: HttpConfiguration,
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn Error>> {
    // Configure tower-http to trace requests/responses
    tracing_subscriber::fmt()
        .with_env_filter("tower_http=debug,libreidp=debug")
        .init();

    let args = Args::parse();
    event!(Level::INFO, "Opening configuration file {}", args.config_file);
    let configuration: Configuration = serde_yaml::from_reader(
        fs::File::open(args.config_file)?)?;

    let mut proxy = IdentityProxy::new(
        &configuration.ldap.uri, configuration.ldap.base).await?;
    let (sender, mut receiver) = mpsc::channel::<oneshot::Sender<
            ldap3::result::Result<String>>>(64);
    tokio::spawn(async move {
        while let Some(responder) = receiver.recv().await {
            responder.send(proxy.query().await).unwrap();
        }
    });

    let app = Router::new()
        .route("/", get({
            let sender = sender.clone();
            || async move {
                let (responder, retriever) = oneshot::channel();
                sender.send(responder).await.unwrap();
                retriever.await.unwrap().map_err(|e| {
                    event!(Level::ERROR, "{:?}", e);
                    StatusCode::INTERNAL_SERVER_ERROR
                })
            }
        }))
        .layer(TraceLayer::new_for_http())
        ;

    let address = configuration.http.address.parse()?;
    event!(Level::INFO, "Starting server on {}", &address);
    axum::Server::bind(&address)
        .serve(app.into_make_service())
        .await?;
    Ok(())
}

///////////////////////////////////////////////////////////////////////////////
