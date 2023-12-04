use bitvec::prelude::*;
use std::env;
use std::fmt;
use std::fs::read_to_string;
use std::ops::Shl;
use std::ops::BitAnd;

type NumberMap = BitArr!(for 256, in u32, Lsb0);

struct Card {
    winning_numbers: NumberMap,
    lot_numbers: NumberMap,
    card_id: usize,
    instances: usize,
}

impl fmt::Display for Card {
    fn fmt(&self, f:& mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "Card({}, instances:{}, winning_numbers:", self.card_id, self.instances)?;
        for i in 0..self.winning_numbers.len() {
            if *self.winning_numbers.get(i).expect("Bit Value") == true {
                write!(f, "{} ", i)?;
            }
        }
        write!(f, "| lot_numbers:")?;
        for i in 0..self.lot_numbers.len() {
            if *self.lot_numbers.get(i).expect("Bit Value") == true {
                write!(f, "{} ", i)?;
            }
        }
        write!(f, ")")
    }

}

impl Card {
    fn parse(line: &str) -> Card {
        let mut ret = Card { card_id: 0, instances: 1, winning_numbers: BitArray::ZERO, lot_numbers: BitArray::ZERO };

        let parts = line.split_whitespace().collect::<Vec<&str>>();
        let mut iter = parts.iter();

        /* First item is word Card */
        if iter.next() != Some(&"Card") {
           panic!("Syntax error '{}'", parts[0]);
        }

        /* Second item defines card ID */
        let card_id_raw = iter.next().expect("Card Id number");
        if card_id_raw.chars().nth_back(0) != Some(':') {
           panic!("Syntax error '{}'", parts[0]);
        }

        let card_id_str = &card_id_raw[0..card_id_raw.len()-1];
        ret.card_id = card_id_str.parse::<usize>().unwrap();

        /* Now parse winning numbers until we reach "|" */
        loop {
            let item = iter.next().expect("New winning number");

            if item == &"|" {
                break;
            }

            let num = item.parse::<usize>().unwrap();
            let mut bit = ret.winning_numbers.get_mut(num).unwrap();
            *bit = true;
        }

        /* And parse the lot numbers */
        loop {
            let item = iter.next();
            if item == None {
                break;
            }

            let num = item.expect("Number").parse::<usize>().unwrap();
            let mut bit = ret.lot_numbers.get_mut(num).unwrap();
            *bit = true;
        }

        return ret;
    }

    fn get_winning_number_count(&self) -> usize {
        self.winning_numbers.bitand(self.lot_numbers).count_ones()
    }

    fn get_winning_points(&self) -> usize {
        let n_matches = self.get_winning_number_count();
        if n_matches > 0 {
            1_usize.shl(n_matches-1)
        } else {
            0_usize
        }
    }
}

fn main() {
    let args: Vec::<String> = env::args().collect();
    if args.len() != 2 {
        println!("Please provide filename to parse");
        std::process::exit(1);
    }

    let mut cards = Vec::<Card>::new();

    for line in read_to_string(args[1].to_string()).unwrap().lines() {
        let trimmed_line = line.trim();
        if ! trimmed_line.is_empty() {
            cards.push(Card::parse(line));
        }
    }

    let mut points = 0_usize;
    for card_idx in 0..cards.len() {
        let winning_numbers;
        let instances;

        {
            let card = &mut cards[card_idx];
            points += card.get_winning_points();
            winning_numbers = card.get_winning_number_count();
            instances = card.instances;
        }

        if winning_numbers > 0 {
            for i in 0..winning_numbers {
                cards[card_idx + i + 1].instances += instances;
            }
        }
    }

    println!("Winning points {}", points);
    let mut num_cards = 0_usize;

    for card_idx in 0..cards.len() {
        let card = &cards[card_idx];

        num_cards += card.instances;
    }

    println!("Number cards {}", num_cards);
}
