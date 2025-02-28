use std::cell::RefCell;
use std::collections::HashMap;
use std::rc::Rc;

use crate::interpreter::ResultType;
use crate::model::Statement;

#[derive(Debug, Clone)]
pub struct State {
    vars: HashMap<String, ResultType>,
    funcs: HashMap<String, Statement>,
    parent: Option<Rc<RefCell<State>>>,
}

impl State {
    pub fn new(parent: Option<Rc<RefCell<State>>>) -> State {
        State {
            vars: HashMap::new(),
            funcs: HashMap::new(),
            parent,
        }
    }

    pub fn get_item(&self, item: &str) -> Option<ResultType> {
        if let Some(value) = self.vars.get(item) {
            return Some(value.clone());
        }

        self.parent
            .as_ref()
            .and_then(|parent| parent.borrow().get_item(item))
    }

    pub fn set_local_item(&mut self, item: String, value: ResultType) {
        self.vars.insert(item, value);
    }

    pub fn set_function(&mut self, item: String, value: Statement) {
        self.funcs.insert(item, value);
    }

    pub fn get_function(&self, item: &str) -> Option<Statement> {
        if let Some(func) = self.funcs.get(item) {
            return Some(func.clone());
        }
        self.parent
            .as_ref()
            .and_then(|parent| parent.borrow().get_function(item))
    }

    pub fn contains(&self, item: &str) -> bool {
        self.vars.contains_key(item)
            || self
                .parent
                .as_ref()
                .map(|parent| parent.borrow().contains(item))
                .unwrap_or(false)
    }

    pub fn set_item(&mut self, item: String, value: ResultType) {
        #[allow(clippy::map_entry)] // known issue on clippy side
        if self.vars.contains_key(&item) {
            self.vars.insert(item, value);
            return;
        }

        if let Some(ref parent) = self.parent {
            let mut parent_ref = parent.borrow_mut();
            if parent_ref.contains(&item) {
                parent_ref.set_item(item, value);
                return;
            }
        }

        self.vars.insert(item, value);
    }

    pub fn get_child_env(state: &Rc<RefCell<State>>) -> State {
        State::new(Some(Rc::clone(state)))
    }
}
