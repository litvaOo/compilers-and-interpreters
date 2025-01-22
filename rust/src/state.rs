use std::collections::HashMap;

use crate::interpreter::ResultType;

#[derive(Debug, Clone)]
pub struct State {
    vars: HashMap<String, ResultType>,
    parent: Option<Box<State>>,
}

impl State {
    pub fn new(parent: Option<Box<State>>) -> State {
        State {
            vars: HashMap::new(),
            parent,
        }
    }

    pub fn get_item(&self, item: String) -> Option<ResultType> {
        if self.vars.contains_key(&item) {
            match self.vars.get(&item) {
                None => return None,
                Some(res) => return Some(res.clone()),
            };
        }
        match self.parent.clone() {
            Some(parent_state) => return parent_state.get_item(item),
            None => return None,
        }
    }

    pub fn set_item(&mut self, item: String, value: ResultType) {
        self.vars.insert(item, value);
    }

    pub fn get_child_env(&self) -> State {
        return State::new(Some(Box::new(self.clone())));
    }
}
