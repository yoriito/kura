# Cargo results
## cargo fmt

```
$ cargo fmt 
$ git status 
branch main
Your branch is up to date with 'origin/main'.

nothing to commit, working tree clean
```


## cargo fix

```
$ cargo fix 
    Checking kura v0.1.0 (/path/to/kura)
    Finished `dev` profile [unoptimized + debuginfo] target(s) in 3.33s
$ git status 
branch main
Your branch is up to date with 'origin/main'.

nothing to commit, working tree clean
```

## cargo clippy

```
$ cargo clippy --fix
    Checking kura v0.1.0 (/path/to/kura)
    Finished `dev` profile [unoptimized + debuginfo] target(s) in 2.56s
$ git status 
branch main
Your branch is up to date with 'origin/main'.

nothing to commit, working tree clean
```

## cargo build
```
$ cargo build 
   Compiling kura v0.1.0 (/path/to/kura)
    Finished `dev` profile [unoptimized + debuginfo] target(s) in 4.37s
```
