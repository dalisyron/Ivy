package imp.ast.expression;

import java.util.Objects;

public record Identifier(String name) {

    public Identifier {
        Objects.requireNonNull(name);

        if (name.isBlank()) {
            throw new IllegalArgumentException("Identifier name cannot be blank");
        }

        if (!Character.isLetter(name.charAt(0))) {
            throw new IllegalArgumentException("Identifier name must start with a letter");
        }

        for (int i = 1; i < name.length(); i++) {
            if (!Character.isLetterOrDigit(name.charAt(i))) {
                throw new IllegalArgumentException("Identifier name must contain only letters and digits");
            }
        }

    }


    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        Identifier that = (Identifier) o;

        return name.equals(that.name);
    }

    @Override
    public int hashCode() {
        return name.hashCode();
    }

    @Override
    public String toString() {
        return name;
    }
}
