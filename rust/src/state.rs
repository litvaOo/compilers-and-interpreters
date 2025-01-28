use std::cell::RefCell;
use std::collections::HashMap;
use std::rc::Rc;

use crate::interpreter::ResultType;

#[derive(Debug, Clone)]
pub struct State {
    vars: HashMap<String, ResultType>,
    parent: Option<Rc<RefCell<State>>>,
}

impl State {
    pub fn new(parent: Option<Rc<RefCell<State>>>) -> State {
        State {
            vars: HashMap::new(),
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

    pub fn set_item(&mut self, item: String, value: ResultType) {
        #[allow(clippy::map_entry)] // known issue on clippy side
        if self.vars.contains_key(&item) {
            self.vars.insert(item, value);
            return;
        }

        if let Some(ref parent) = self.parent {
            let mut parent_ref = parent.borrow_mut();
            if parent_ref.vars.contains_key(&item) {
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
